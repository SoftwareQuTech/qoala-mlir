#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "Conversion/Helpers/Helpers.h"

#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "angletransformhelper"


namespace qoala::helpers::angle {

    std::string angleConversionFunctionName("__qoala_convert_float_angle");

    bool moduleContainsAngleConversionDeclaration(ModuleOp &module) {
        auto functionDeclaration = module.lookupSymbol<func::FuncOp>(angleConversionFunctionName);
        return functionDeclaration;
    }

    Operation *insertAngleConversionFunctionDeclaration(ModuleOp &module) {
        OpBuilder builder(module.getBodyRegion());
        FunctionType functionType = builder.getFunctionType(
                builder.getF32Type(),
                {builder.getI32Type(), builder.getI32Type()});
        auto funcDeclaration = builder.create<func::FuncOp>(
                module->getLoc(),
                StringRef{angleConversionFunctionName},
                functionType);
        // We set an attribute on the function, so we can recognize it later
        Attribute attr = builder.getStringAttr("true");
        funcDeclaration->setAttr("qoala-builtin", attr);
        // Function declarations must have the private visibility
        funcDeclaration.setVisibility(func::FuncOp::Visibility::Private);
        return funcDeclaration;
    }

    /**
     * Simple function to compute an angle in radians using two integers "n" and "e".
     * The returned double value "a" follows the formula: a=\frac{n\times\pi}{2^e}
     * @param n the n integer
     * @param e the e integer
     * @return the value of the angle in radians
     */
    static double angleCalculator(const uint32_t n, const uint32_t e) {
        // For the reader. Why the denom of this computation is "1 << e"
        // instead of "std::pow(2, e)"??
        return static_cast<double>(n) * M_PI / static_cast<double>(1 << e);
    }

#define MAX_SEARCH_ITERATIONS 200
#define ALMOST_PERFECT_THRESHOLD 1.0e-6

    /**
     * Given an angle "a" in radians, search for two integers n and e such that
     * a = f(n,e) = \frac{n\times\pi}{2^e}.
     * The implementation follows a similar approach w.r.t the Newton-Raphson algorithm.
     * The given angle can be associated to a contour curve on the 2-dimensional
     * sheet a = f(n,e). The idea is to find that contour curve, and follow it discretely
     * (varying n and e as integers) for some time (i.e. limited number of iterations)
     * until we find the best match for the given angle value.
     * @param angleRads an angle (type double) expressed in radians
     * @return a vector of two integers (n, e)
     */
    std::vector<uint32_t> transformDouble(const double /*a=*/angleRads) {
        // This algorithm is based on some observations:
        // * The contour curve has a logarithmic shape. In fact, assuming that "a"
        //   is constant, it is not difficult to compute the contour curve for that a:
        //   e = f(n) = \frac{1}{ln 2}\times ln(n) + \frac{ln \pi - ln a}{ln 2}
        //   This curve can be visualized as "taking a picture from the z axis"
        // * Since the contour curve is logarithmic, lim_{n -> \infty} f(n) = \infty,
        //   hence it is not possible to iterate indefinitely and "automatically stop"
        //   when we reach certain threshold; we fix the maximum number of the search
        //   iterations as MAX_SEARCH_ITERATIONS
        // * As a heuristic to improve the speed of the algorithm, we use the
        //   "ALMOST_PERFECT_THRESHOLD" value. We use it to test the "fitness" of the
        //   found integer pair to eagerly stop searching. The given integer pair yields
        //   an angle that is so close that we can call it "good enough".
        // * The search starts on the n axis (e = 0), at ceil(a/3). This limit can be
        //   computed from the general from of the angle.
        // * The idea is to follow the contour curve, moving only in one axis until we
        //   cross the contour curve. We also keep track of the best approximation found
        //   so far (bestN, bestE, bestThreshold)
        // * Every time we cross the curve, we test the new approximation, comparing it
        //   with the best results so far.

        auto n = static_cast<uint32_t>(std::ceil(angleRads / 3.0));
        uint32_t e = 0, bestN = 0, bestE = 0;
        double bestThreshold = 0.0;

        // If angle is 0, then angleCalculator(0, 0) == 0.0, so we simply return.
        if (angleRads != angleCalculator(n, e)) {
            // The initial guess of n should yield an angle value >= than the given one,
            // We move over the n axis (lower n) until we cross the contour line i.e. we get
            // an angle value less than the given one.
            while (angleRads <= angleCalculator(n, e)) {
                n--;
            }
            // In this point we know that angleCalculator(n, 0) < angleRads < angleCalculator(n+1, 0)
            // these are our best results so far:
            n = n + 1;
            bestN = n;
            bestE = e;
            bestThreshold = std::abs(angleCalculator(bestN, bestE) - angleRads);

            // We discretely follow the log curve, trying to find a point that satisfies the given tolerance
            for (uint32_t i = 0; i < 2 * MAX_SEARCH_ITERATIONS || bestThreshold == 0.0; i++) {
                LLVM_DEBUG(llvm::dbgs() << "Status: (" << bestN << ", " << bestE << ", " << bestThreshold<< ")\n");
                double currentAngle = angleCalculator(n, e);

                if (currentAngle == angleRads || bestThreshold <= ALMOST_PERFECT_THRESHOLD) {
                    // Jackpot! We found a (almost) perfect match
                    break;
                }

                while (angleRads <= currentAngle) {
                    // On even iterations, we move over e, on odd iterations we move over n
                    i % 2 == 0 ? e++ : n++;
                    currentAngle = angleCalculator(n, e);
                    // Distance between the current set of values, and the given angle
                    double threshold = std::abs(currentAngle - angleRads);
                    if (threshold < bestThreshold) {
                        // We found a better fit for the approximation
                        bestThreshold = threshold;
                        bestN = n;
                        bestE = e;
                    }
                }
            }
        }
        LLVM_DEBUG(llvm::dbgs() << "Returning: (" << bestN << ", " << bestE << ")\n");
        return {bestN, bestE};
    }
}
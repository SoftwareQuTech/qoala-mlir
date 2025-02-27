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
     * sheet a = f(n,e). The idea is to find that contour curve, and follow it for
     * some time (limited number of iterations) until we find the best match for the
     * given angle value.
     * @param angleRads an angle (type double) expressed in radians
     * @return a vector of two integers (n, e)
     */
    std::vector<uint32_t> transformDouble(const double /*a=*/angleRads) {
        // This algorithm is based on some observations:
        // * The contour curve has a logarithmic shape. In fact, assuming that "a"
        //   is constant, it is not difficult to compute the contour curve:
        //   e = f(n) = \frac{1}{ln 2}\times ln(n) + \frac{ln a + ln 1}{ln 2}
        //   This curve can be visualized as "taking a picture from the z axis"
        // * Since the contour curve is logarithmic, lim_{n -> \infty} f(n) = \infty,
        //   hence it is not possible to iterate indefinitely and "automatically stop"
        //   when we reach certain threshold; we fix the maximum number of the search
        //   iterations as MAX_SEARCH_ITERATIONS
        // * As a heuristic to improve the speed of the algorithm, we use the
        //   "ALMOST_PERFECT_THRESHOLD" value. We use it to test the "fitness" of the
        //   found integer pair to eagerly stop searching. The given integer pair yields
        //   an angle that is so close that we can call it "good enough".

        // TODO - Keep documenting this function.
        auto n = static_cast<uint32_t>(std::ceil(angleRads / 3.0));
        uint32_t e = 0, bestN = 0, bestE = 0;
        double bestThreshold = 0.0;

        if (angleRads != angleCalculator(n, e)) {
            // The given angle is not 0.0
            while (angleRads <= angleCalculator(n, e)) {
                n--;
            }
            // In this point we know that angleCalculator(n, 0) < angleRads < angleCalculator(n+1, 0)
            // Starting point of thew approximation; these are our best results so far:
            n = n + 1;
            bestN = n;
            bestE = e;
            bestThreshold = std::abs(angleCalculator(bestN, bestE) - angleRads);

            // We discretely follow the log curve, trying to find a point that satisfies the given tolerance
            for (uint32_t i = 0; i < 2 * MAX_SEARCH_ITERATIONS || bestThreshold == 0.0; i++) {
                LLVM_DEBUG(llvm::dbgs() << "Status: (" << bestN << ", " << bestE << ", " << bestThreshold<< ")\n");
                double currentAngle = angleCalculator(n, e);

                if (currentAngle == angleRads || bestThreshold <= ALMOST_PERFECT_THRESHOLD) {
                    // Jackpot! We found a perfect match
                    break;
                }

                while (angleRads <= currentAngle) {
                    // On even iterations, we move over e, on odd iterations we move over n
                    i % 2 == 0 ? e++ : n++;
                    currentAngle = angleCalculator(n, e);
                    double distance = std::abs(currentAngle - angleRads);
                    if (distance < bestThreshold) {
                        bestThreshold = distance;
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
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "Conversion/Helpers/Helpers.h"

#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "angletransformhelper"

using namespace mlir;

namespace qoala::helpers::angle {

    std::string angleConversionFunctionName("__qoala_convert_float_angle");

    bool moduleContainsAngleConversionDeclaration(ModuleOp &module) {
        return module.lookupSymbol<func::FuncOp>(angleConversionFunctionName);
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
    static double getAngleInRads(const uint32_t n, const uint32_t e) {
        // For the reader. Why the denom of this computation is "1 << e"
        // instead of "std::pow(2, e)"??
        assert(e < 32 && "exponent must not be >= 32");
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
        // * The idea is to follow the contour curve discretely:
        //   1. First, we approach to the curve starting from n=ceil(a/3), going down until
        //      we cross the curve.
        //   2. Once we cross, we assume the lower point is our first approximation, and
        //      we start searching for better ones.
        //   3. We fix n, and start exploring values of e, until we cross the curve again.
        //   4. Once we cross the curve, we evaluate both points that are close to the curve:
        //      (n, e) and (n, e-1). If any of them is a best approximation, we make note of it.
        //   5. We continue searching by moving to the right (n = n+1) repeat from step 3
        //      knowing that the e value must not be lower that the one we found in the last
        //      iteration. This can be assumed because the contour curve grows monotonically.
        //   6. We repeat steps 3 to 5, until we find an approximation that is "good enough"
        //      or we exhaust the maximum number of tries.

        LLVM_DEBUG(llvm::dbgs() << "Transforming: " << angleRads << "\n");

        auto n = static_cast<uint32_t>(std::ceil(angleRads / 3.0));
        uint32_t bestN = n, bestE = 0;

        // If angle is 0, then angleCalculator(0, 0) == 0.0, so we simply return.
        if (angleRads != getAngleInRads(n, 0)) {
            // The initial guess of n should yield an angle value >= than the given one,
            // We move over the n axis (lowering n, e == 0) until we cross the contour line
            // i.e. we get an angle value less than the given one.
            while (angleRads <= getAngleInRads(n, 0)) {
                n--;
            }
            // In this point we know that angleCalculator(n, 0) < angleRads < angleCalculator(n+1, 0)
            // these are our best results so far:
            n = n + 1;
            bestN = n;
            bestE = 0;
            uint32_t highestE = bestE;
            double currentAngle = getAngleInRads(bestN, bestE);
            double bestApproxAngle = currentAngle;
            double bestThreshold = std::abs(currentAngle - angleRads);

            // Once that we found the first crossing of the contour curve, we will iterate at most
            // MAX_SEARCH_ITERATIONS values of "n" to the right.
            for (; n < MAX_SEARCH_ITERATIONS; n++) {
                LLVM_DEBUG(llvm::dbgs() << "Status: (" << n << ", " << bestN << ", " << bestE << ", " << bestThreshold << ", " << currentAngle << ")\n");
                // Iterate over all the exponents, starting at the highest exponent so far, trying to cross the contour curve
                // NOTE: we limit the exponent to < 32, so we don't overflow the denom of angleCalculator
                uint32_t e;
                double lastCurrentAngle = currentAngle;
                for (e = highestE; e < 32; e++) {
                    lastCurrentAngle = currentAngle; //angle at (n, e-1)
                    currentAngle = getAngleInRads(n, e); // angle at (n, e)
                    if (currentAngle <= angleRads) {
                        // We crossed the contour curve, make note of this point
                        highestE = e - 1;
                        break;
                    }
                }

                // The contour curve passes between (n, e -1), (n, e):
                // Evaluate the distance on both points, and update the best found so far
                double thresholdDown = std::abs(lastCurrentAngle - angleRads);
                double thresholdUp = std::abs(currentAngle - angleRads);
                if (thresholdDown < bestThreshold) {
                    bestN = n;
                    bestE = e - 1;
                    bestApproxAngle = lastCurrentAngle;
                    bestThreshold = thresholdDown;
                }
                if (thresholdUp < bestThreshold) {
                    bestN = n;
                    bestE = e;
                    bestApproxAngle = currentAngle;
                    bestThreshold = thresholdUp;
                }

                // Check if the search is good enough
                if (bestApproxAngle == angleRads || bestThreshold <= ALMOST_PERFECT_THRESHOLD) {
                    // Jackpot! We found a (almost) perfect match
                    break;
                }
            }
        }
        LLVM_DEBUG(llvm::dbgs() << "Returning: (" << bestN << ", " << bestE << ")\n************************\n");
        return {bestN, bestE};
    }
}
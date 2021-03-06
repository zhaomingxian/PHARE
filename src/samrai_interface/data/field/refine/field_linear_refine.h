#ifndef PHARE_FIELD_LINEAR_REFINE_H
#define PHARE_FIELD_LINEAR_REFINE_H


#include "data/field/field.h"
#include "data/grid/gridlayoutdefs.h"
#include "linear_weighter.h"
#include "utilities/constants.h"
#include "utilities/point/point.h"

#include <SAMRAI/hier/Box.h>

#include <array>
#include <utility>
#include <vector>


namespace PHARE
{
namespace amr_interface
{
    using core::dirX;
    using core::dirY;
    using core::dirZ;

    template<std::size_t dimension>
    class FieldRefineIndexesAndWeights
    {
    public:
        /** @brief Given a centering in each directions and a ratio, initialize weights and shifts
         *  for later use. (the vector of weights will be the same regardless of the fineIndex)
         * it is which index of the weights that will be used depends on the fineIndex, and
         * also which coarseIndex to start for refine operation
         *
         */
        FieldRefineIndexesAndWeights(std::array<core::QtyCentering, dimension> centerings,
                                     SAMRAI::hier::IntVector const& ratio)
            : ratio_{ratio}
            , weighters_{make_weighters(centerings, ratio, std::make_index_sequence<dimension>{})}

        {
            // this shift will be use to determine which coarseIndexe we take
            for (std::size_t iDir = dirX; iDir < dimension; ++iDir)
            {
                if (centerings[iDir] == core::QtyCentering::primal)
                {
                    shifts_[iDir] = 0.;
                }
                else
                {
                    // in case we are dual, we need to shift our fine index of - halfRatio
                    // so that after truncating to integer (the index/ratio), we get the correct
                    // coarseStartIndex
                    shifts_[iDir] = 0.5;
                }
            }
        }




        core::Point<int, dimension> coarseStartIndex(core::Point<int, dimension> fineIndex) const
        {
            core::Point<int, dimension> coarseIndex{fineIndex};

            // here we perform the floating point division, and then we truncate to integer
            coarseIndex[dirX] = static_cast<int>(
                static_cast<double>(fineIndex[dirX] + shifts_[dirX]) / ratio_(dirX)
                - shifts_[dirX]);

            if constexpr (dimension > 1)
            {
                coarseIndex[dirY] = static_cast<int>(
                    static_cast<double>(fineIndex[dirY] + shifts_[dirY]) / ratio_(dirY)
                    - shifts_[dirY]);
            }

            if constexpr (dimension > 2)
            {
                coarseIndex[dirZ] = static_cast<int>(
                    static_cast<double>(fineIndex[dirZ] + shifts_[dirZ]) / ratio_(dirZ)
                    - shifts_[dirZ]);
            }

            return coarseIndex;
        }




        typename LinearWeighter::FineIndexWeights const& weights(core::Direction dir) const
        {
            return weighters_[static_cast<std::size_t>(dir)].weights();
        }




        /** @brief Compute the index of weigths for a given fineIndex
         */
        core::Point<int, dimension> computeWeightIndex(core::Point<int, dimension> fineIndex) const
        {
            core::Point<int, dimension> indexesWeights{fineIndex};

            indexesWeights[dirX] %= ratio_[dirX];

            if constexpr (dimension > 1)
            {
                indexesWeights[dirY] %= ratio_[dirY];
            }
            if constexpr (dimension > 2)
            {
                indexesWeights[dirZ] %= ratio_[dirZ];
            }

            return indexesWeights;
        }

    private:
        SAMRAI::hier::IntVector const ratio_;
        std::array<LinearWeighter, dimension> weighters_;
        core::Point<double, dimension> shifts_;
    };

} // namespace amr_interface

} // namespace PHARE

#endif

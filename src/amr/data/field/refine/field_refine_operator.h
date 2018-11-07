#ifndef PHARE_FIELD_REFINE_OPERATOR_H
#define PHARE_FIELD_REFINE_OPERATOR_H

#include "data/field/field_data.h"
#include "data/field/field_geometry.h"
#include "data/grid/gridlayout.h"
#include "field_linear_refine.h"
#include "field_refiner.h"

#include <SAMRAI/hier/RefineOperator.h>
#include <SAMRAI/tbox/Dimension.h>

#include <string>


namespace PHARE
{
template<typename GridLayoutT, typename FieldT>
class FieldRefineOperator : public SAMRAI::hier::RefineOperator
{
public:
    using GridLayoutImpl                   = typename GridLayoutT::implT;
    using PhysicalQuantity                 = typename FieldT::physical_quantity_type;
    static constexpr std::size_t dimension = GridLayoutT::dimension;

    FieldRefineOperator()
        : SAMRAI::hier::RefineOperator{"FieldRefineOperator"}
    {
    }

    virtual ~FieldRefineOperator() = default;

    /** This implementation have the top priority for refine operation
     *
     */
    int getOperatorPriority() const override { return 0; }

    /**
     * @brief This operator need to have at least 1 ghost cell to work properly
     *
     */
    SAMRAI::hier::IntVector getStencilWidth(SAMRAI::tbox::Dimension const& dim) const override
    {
        return SAMRAI::hier::IntVector::getOne(dim);
    }




    /**
     * @brief Given a  set of box on a  fine patch, compute the interpolation from
     * a coarser patch that is underneath the fine box.
     * Since we get our boxes from a FieldOverlap, we know that there are in correct
     * Field Indexes
     *
     */
    void refine(SAMRAI::hier::Patch& destination, SAMRAI::hier::Patch const& source,
                int const destinationComponent, int const sourceComponent,
                SAMRAI::hier::BoxOverlap const& destinationOverlap,
                SAMRAI::hier::IntVector const& ratio) const override
    {
        auto const& destinationFieldOverlap
            = dynamic_cast<FieldOverlap<dimension> const&>(destinationOverlap);

        auto const& overlapBoxes = destinationFieldOverlap.getDestinationBoxContainer();

        auto& destinationField        = getField_(destination, destinationComponent);
        auto const& destinationLayout = getLayout_(destination, destinationComponent);

        auto const& sourceField  = getField_(source, sourceComponent);
        auto const& sourceLayout = getLayout_(source, sourceComponent);


        // We assume that quantity are the same
        // note that an assertion will be raised
        // in refineIt operator
        auto const& qty = destinationField.physicalQuantity();

        bool const withGhost{true};

        auto destinationFieldBox = FieldGeometry<GridLayoutT, PhysicalQuantity>::toFieldBox(
            destination.getBox(), qty, destinationLayout, withGhost);

        auto sourceFieldBox = FieldGeometry<GridLayoutT, PhysicalQuantity>::toFieldBox(
            source.getBox(), qty, sourceLayout, withGhost);




        FieldRefiner<dimension> refine{destinationLayout.centering(qty), destinationFieldBox,
                                       sourceFieldBox, ratio};



        for (auto const& box : overlapBoxes)
        {
            // we compute the intersection with the destination,
            // and then we apply the refine operation on each fine
            // index.
            auto intersectionBox = destinationFieldBox * box;




            if constexpr (dimension == 1)
            {
                int iStartX = intersectionBox.lower(dirX);
                int iEndX   = intersectionBox.upper(dirX);

                for (int ix = iStartX; ix <= iEndX; ++ix)
                {
                    refine(sourceField, destinationField, {{ix}});
                }
            }




            else if constexpr (dimension == 2)
            {
                int iStartX = intersectionBox.lower(dirX);
                int iStartY = intersectionBox.lower(dirY);

                int iEndX = intersectionBox.upper(dirX);
                int iEndY = intersectionBox.upper(dirY);

                for (int ix = iStartX; ix <= iEndX; ++ix)
                {
                    for (int iy = iStartY; iy <= iEndY; ++iy)
                    {
                        refine(sourceField, destinationField, {{ix, iy}});
                    }
                }
            }




            else if constexpr (dimension == 3)
            {
                int iStartX = intersectionBox.lower(dirX);
                int iStartY = intersectionBox.lower(dirY);
                int iStartZ = intersectionBox.lower(dirZ);

                int iEndX = intersectionBox.upper(dirX);
                int iEndY = intersectionBox.upper(dirY);
                int iEndZ = intersectionBox.upper(dirZ);

                for (int ix = iStartX; ix <= iEndX; ++ix)
                {
                    for (int iy = iStartY; iy <= iEndY; ++iy)
                    {
                        for (int iz = iStartZ; iz <= iEndZ; ++iz)
                        {
                            refine(sourceField, destinationField, {{ix, iy, iz}});
                        }
                    }
                }
            }
        }
    }

private:
    GridLayoutT const& getLayout_(SAMRAI::hier::Patch const& patch, int id) const
    {
        auto const& patchData
            = std::dynamic_pointer_cast<FieldData<GridLayoutT, FieldT>>(patch.getPatchData(id));
        if (!patchData)
        {
            throw std::runtime_error("cannot cast to FieldData");
        }
        return patchData->gridLayout;
    }


    FieldT& getField_(SAMRAI::hier::Patch const& patch, int id) const
    {
        auto const& patchData
            = std::dynamic_pointer_cast<FieldData<GridLayoutT, FieldT>>(patch.getPatchData(id));
        if (!patchData)
        {
            throw std::runtime_error("cannot cast to FieldData");
        }
        return patchData->field;
    }
};

} // namespace PHARE



#endif

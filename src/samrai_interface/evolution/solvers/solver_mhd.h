
#ifndef PHARE_SOLVER_MHD_H
#define PHARE_SOLVER_MHD_H

#include "evolution/messengers/mhd_messenger_info.h"
#include "evolution/solvers/solver.h"

namespace PHARE
{
namespace amr_interface
{
    template<typename MHDModel>
    class SolverMHD : public ISolver
    {
    public:
        SolverMHD()
            : ISolver{"MHDSolver"}
        {
        }


        virtual ~SolverMHD() = default;

        virtual std::string modelName() const override { return MHDModel::model_name; }


        virtual void fillMessengerInfo(std::unique_ptr<IMessengerInfo> const& info) const override
        {
            //
        }


        virtual void registerResources(IPhysicalModel& model) override {}

        // TODO make this a resourcesUser
        virtual void allocate(IPhysicalModel& model, SAMRAI::hier::Patch& patch,
                              double const allocateTime) const override
        {
        }

        virtual void advanceLevel(std::shared_ptr<SAMRAI::hier::PatchHierarchy> const& hierarchy,
                                  int const levelNumber, IPhysicalModel& model,
                                  IMessenger& fromCoarser, const double currentTime,
                                  const double newTime) override
        {
        }
    };
} // namespace amr_interface
} // namespace PHARE


#endif

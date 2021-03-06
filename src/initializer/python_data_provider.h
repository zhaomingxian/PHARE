#ifndef PHARE_PYTHON_DATA_PROVIDER_H
#define PHARE_PYTHON_DATA_PROVIDER_H

#include "data_provider.h"

#include <pybind11/embed.h> // everything needed for embedding
#include <pybind11/functional.h>

namespace py = pybind11;

//#include "models/physical_state.h"
//#include <data/ions/ions.h>

namespace PHARE
{
namespace initializer
{
    extern py::scoped_interpreter guard;



    class PythonDataProvider : public DataProvider
    {
    public:
        PythonDataProvider(int argc, char const* argv)
        {
            if (argc == 2)
            {
                initModuleName_ = argv[1];
            }
            preparePythonPath_();
        }

        /**
         * @brief read overrides the abstract DataProvider::read method. This method basically
         * executes the user python script that fills the dictionnary.
         */
        virtual void read() override
        {
            py::eval_file(initModuleName_, scope_);
            auto& d                      = dict<1>();
            auto& pop1                   = d["simulation"]["hybridstate"]["ions"]["pop1"];
            auto popname                 = pop1["name"].to<std::string>();
            auto particleInitializerName = pop1["particleinitializer"]["name"].to<std::string>();
            auto& density = pop1["particleinitializer"]["density"].to<ScalarFunction<1>>();
            std::cout << "pop1 (" + popname + ") has a " + particleInitializerName
                             + "particle initializer"
                      << " and density(2) = " << density(2.) << "\n";
        }


    private:
        void preparePythonPath_() { py::eval_file("setpythonpath.py", scope_); }


        std::string initModuleName_{"job.py"};
        py::scoped_interpreter guard_{};
        py::object scope_{py::module::import("__main__").attr("__dict__")};
    };

} // namespace initializer

} // namespace PHARE

#endif // DATA_PROVIDER_H

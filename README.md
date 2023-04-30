# Fair-Division

### Description:
The code is a modified implementation of [[https://dl.acm.org/doi/10.1145/3219166.3219176]](https://dl.acm.org/doi/10.1145/3219166.3219176) and [[https://ojs.aaai.org/index.php/AAAI/article/view/16703]](https://ojs.aaai.org/index.php/AAAI/article/view/16703). 
* Different versions (folders) represent significant code structure changes or attempts at multiple modifications (see respective folder README.md). 
* Codes for chores and goods have been separated.
* For testing, we generate pseudorandom number generators for Uniform, Normal, Exponential and Similar (all agents have similar valuations for same item) distributions. This can be set in `main.cpp` along with required parameters for each.
* The number of testing samples, upper and lower bounds on valuations, #agents and #items can also be set in `main.cpp`
* Contact: Vanshika Gupta (vg18@illinois.edu or vanshika421@gmail.com)
* Advisors: [Prof Rakesh Nagi](https://rakeshnagi.ise.illinois.edu/), [Prof Jugal Garg](https://jugal.ise.illinois.edu/)

### Steps to run the Market Based Algorithm 
* Requirements: Make, Sciplot (https://sciplot.github.io/), PCG RNG (https://www.pcg-random.org/download.html)
    * For Sciplot: g++ -std=c++17 is required (Specifies the C++ version or ISO standard version)
    * If graph generation using Sciplot not required, comment `#include <sciplot/sciplot.hpp>` from `output.cpp` and function `drawVerificationCurve` in `output.h`, `output.cpp` and `main.cpp`
    * Download the PCG Random Number generator library and add the files to the the default C++ `include` folder
    * If `Make` not installed, copy past makefile.mak contents sequentilly in terminal to compile.
* Clone the repository (download zip or clone)
* Set the number of samples to generate, upper and lower bounds on valuations, #agents and #items in `main.cpp`
* Run `make -f makefile.mak`. If make command not supported either install `make` or run `g++ main.cpp helper.cpp output.cpp`
* Run the exexutive file generated - `./main`
* To log the console output to a log file, run `./main | tee "/<fileLocation>/output.txt"`


### Steps to run the Mixed Integer Programming Based Algorithm
* Requirements: Gurobi, Sciplot, PCG RNG (see requirements in [above section](#steps-to-run-the-market-based-algorithm)),
* Clone the repository (download zip or clone)
* Set the number of samples to generate, upper and lower bounds on valuations, #agents and #items in `main.cpp`
* Run `g++ -I/opt/gurobi952/linux64/include/ -L/opt/gurobi952/linux64/lib helper.o output.o -o EF1_LP EF1_LP.cpp -lgurobi_c++ -lgurobi95`, 
* Details: `-I` option appends specified directory to the list of possible drectories to search for `includes`. `-L[/path/to/shared-libraries] ` option specifies the Gurobi shared libraries to use. This is required when building or using a shared library like libstdc++.so See more details: [Compiler-Flags-Options](https://caiorss.github.io/C-Cpp-Notes/compiler-flags-options.html), [Gurobi Guide](https://www.gurobi.com/documentation/9.5/quickstart_linux/cpp_building_and_running_t.html)
* Run the exexutive file generated - `./EF1_LP`
* To log the console output to a log file, run `./EF1_LP | tee "/<fileLocation>/output.txt"`



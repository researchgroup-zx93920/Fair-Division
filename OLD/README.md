# Fair-Division

### Description:
The code is a modified implementation of [[https://dl.acm.org/doi/10.1145/3219166.3219176]](https://dl.acm.org/doi/10.1145/3219166.3219176) and [[https://ojs.aaai.org/index.php/AAAI/article/view/16703]](https://ojs.aaai.org/index.php/AAAI/article/view/16703). 
* Different versions (folders) represent significant code structure changes or attempts at multiple modifications (see respective folder README.md). 
* Codes for chores and goods have been separated.
* For testing, we generate pseudorandom number generators for Uniform, Normal, Exponential and Similar (all agents have similar valuations for same item) distributions. This can be set in `main.cpp` along with required parameters for each.
* The number of testing samples, upper and lower bounds on valuations, #agents and #items can also be set in `main.cpp`
* Contact: Vanshika Gupta (vg18@illinois.edu or vanshika421@gmail.com)
* Advisors: [Prof Rakesh Nagi](https://rakeshnagi.ise.illinois.edu/), [Prof Jugal Garg](https://jugal.ise.illinois.edu/)

### Steps to run the code
* Clone the repository (download zip or clone)
* Set the number of samples to generate, upper and lower bounds on valuations, #agents and #items in `main.cpp`
* Run `make -f makefile.mak`. If make command not supported, run `g++ main.cpp helper.cpp output.cpp`
* Run the exexutive file generated - `./main`
* To log the console output to a log file, run `./main | tee "/<fileLocation>/output.txt"`

#### latest code: V2.1-Chores



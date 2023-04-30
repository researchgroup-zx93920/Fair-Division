# Fair-Division

#### Steps to run the code
* This is the first attempt to resolve the cycling problem. 
* We increase the price of only the first least spenders component if there is no alternating path from LS to PV. 
* If there are multiple LS, increase the price by an amount such that it does not remain the LS and nothing else.
* If there is only 1, increase to add an MBB edge.
* 1st workaround - Check the ratio such that an agent in the LS component does not become the BS. 
* Outcome - failing due to some random error.

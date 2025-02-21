# Using onnxruntime llm model and integrate it into flutter.
Simulating token generation require kinds of coroutine. The flutter's one quite different with python or c++. I had struggled with solving some errors.

***
* My coroutine function not work well even after I checked the listener that attached this function work well.
![01_2025_feb_001_integrate_to_flutter_not_work_well_coroutine](./images/01_2025_feb_001_integrate_to_flutter_not_work_well_coroutine.png)

***
* First time I faced error it seems the streamcontroller needed to be broadcast one to get multiple listener to use streambuilder. But it still not working.
![02_2025_feb_001_integrate_to_flutter_not_work_well_coroutine_problem1_1](./images/02_2025_feb_001_integrate_to_flutter_not_work_well_coroutine_problem1_1.png)


***
* I expand debugging area from my workspace to almost whole libraries. and I found something went wrong.
![03_2025_feb_001_integrate_to_flutter_not_work_well_coroutine_problem1_2](./images/03_2025_feb_001_integrate_to_flutter_not_work_well_coroutine_problem1_2.png)

***
* I made two functions to check what is wrong.
![04_2025_feb_001_integrate_to_flutter_not_work_well_coroutine_problem1_3](./images/04_2025_feb_001_integrate_to_flutter_not_work_well_coroutine_problem1_3.png)

***
* I finally found what's the problem. It works quite differently with other languages. And this information I could'n get easily.
![05_2025_feb_001_integrate_to_flutter_not_work_well_coroutine_solve1](./images/05_2025_feb_001_integrate_to_flutter_not_work_well_coroutine_solve1.png)

***
* My app now able to simulate print each tokens whenever each of them generated.
![06_2025_feb_001_integrate_to_flutter_not_work_well_coroutine_solve2](./images/06_2025_feb_001_integrate_to_flutter_not_work_well_coroutine_solve2.png)

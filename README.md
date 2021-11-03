# Nova 2D Game Engine

Idea:
	I want to make the whole engine in c++ but let the games be creatable in javascript/typescript while still be able to create the whole game in c/c++. This way the heavy computations can be performed natively but games can quickly be made with a higher lever language like js/ts.

Features:
- Embedded Google's V8 Javascript Engine ✅
- Add Game Engine logic (GameLoop, State, Script Execution)
- Fiber Based Task system (Multi threaded)
- Move Parallel computations to Compute Shaders
- Use Vulkan for accelerated GPU drawing ✅
- Port Libraries to the game (javascript) side

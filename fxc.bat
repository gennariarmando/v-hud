for %%f in ("./shaders/*.hlsl") do fxc /T ps_2_0 /Fo "./shaders/%%~nf.fxc" "./shaders/%%~nf.hlsl"

# Run with: python3 build.py
import os
import platform

# (1)==================== COMMON CONFIGURATION OPTIONS ======================= #
COMPILER="g++ -g -std=c++17"   # The compiler we want to use 
                                #(You may try g++ if you have trouble)
SOURCE="./src/*.cpp"    # Where the source code lives
EXECUTABLE="project"        # Name of the final executable
# ======================= COMMON CONFIGURATION OPTIONS ======================= #

# (2)=================== Platform specific configuration ===================== #
# For each platform we need to set the following items
ARGUMENTS=""            # Arguments needed for our program (Add others as you see fit)
INCLUDE_DIR=""          # Which directories do we want to include.
LIBRARIES=""            # What libraries do we want to include

if platform.system()=="Linux":
    ARGUMENTS="-D LINUX" # -D is a #define sent to preprocessor
    INCLUDE_DIR="-I ./include/ -I ./../common/thirdparty/glm/"
    LIBRARIES="-lSDL2 -ldl"
elif platform.system()=="Darwin":
    ARGUMENTS="-D MAC" # -D is a #define sent to the preprocessor.
    INCLUDE_DIR="-I ./include/ -I/Library/Frameworks/SDL2.framework/Headers -I./../common/thirdparty/old/glm"
    LIBRARIES="-F/Library/Frameworks -framework SDL2"
elif platform.system()=="Windows":
    ARGUMENTS="-D MINGW -static-libgcc -static-libstdc++" 
    INCLUDE_DIR="-I./include/ -I./../common/thirdparty/old/glm/"
    EXECUTABLE="lab.exe"
    LIBRARIES="-lmingw32 -lSDL2main -lSDL2 -mwindows"
# (2)=================== Platform specific configuration ===================== #

# (3)====================== Building the Executable ========================== #
# Build a string of our compile commands that we run in the terminal
compileString=COMPILER+" "+ARGUMENTS+" "+SOURCE+" -o "+EXECUTABLE+" "+" "+INCLUDE_DIR+" "+LIBRARIES
# Print out the compile string
# This is the command you can type
print("===============================================================================")
print("====================== Compiling on: "+platform.system()+" =============================")
print("===============================================================================")
print(compileString)
print("===============================================================================")
# Run our command 
os.system(compileString)
# ========================= Building the Executable ========================== #

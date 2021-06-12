#include "../Lightning/Lightning.hpp"

using namespace Lightning;

int main()
{
  // Any one time startup and static initialization Lightning needs to do
  // This also registers a custom assertion handler (Lightning code has many user friendly asserts!)
  LightningSetup setup(SetupFlags::None);

  // A project contains all of the code we combine together to make a single Lightning library
  // The project also sends events for compilation errors that occur (includes friendly messages / error codes)
  Project project;

  // Here, we can register our own callback for when compilation errors occur
  // The default callback prints the file, line/character number, and message to stderr
  EventConnect(&project, Events::CompilationError, DefaultErrorCallback);

  // Add some test Lightning code that does NOT compile, just to try it out!
  // The second parameter provides a name for when compilation errors come up, in this case 'MyCode'
  project.AddCodeFromString("class Foozle++", "MyCode");

  // Create a list of dependent libraries, in our case we're really not adding anything to this
  // A side note: the Core library in Lightning is always added as a dependency, because Core includes
  // things like Integer, Boolean, Real, the basic vector types, String, etc
  Module dependencies;

  // Compile all the code we added together into a single library named "Game"
  // We already know this is going to fail to compile, so the error callback
  // we provided above should get called
  project.Compile("Game", dependencies, EvaluationMode::Project);

  return 0;
}

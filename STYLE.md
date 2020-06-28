# Code style guidelines

BurekTech's style is pretty straightforward.  
 
## Do's

Newlines after if statements, with spaces between the if and brackets, and the condition and brackets:
```
if ( condition )
{
	// code
}
``` 
 
Spaces in function parameters:
```
// With parameters
void DoSomething( int value );

// Without parameters
void DoNothing();

// Calling each
DoSomething( 5 );
DoNothing();
```

Have some tabs in classes:
```
class Example
{
public:
	void			Spawn();
	void 			Use();
	void 			Die();

private:
	int 			lives;
	float			underwaterTime; 			
}
```
 
Templates would ideally have spaces like this:
```
template<typename T>
void DoSomething( T value );

// ...

player = static_cast<BasePlayer*>( entity );
```
Hopefully you're getting acquainted with BurekTech's space philosophy by now. Whenever in doubt, look at the existing code! 
 
Function names: PascalCase 
Variable names: camelCase 
Class names: PascalCase_UnderlineTolerant
Struct names: camelCase_UnderlineTolerant

Each class should be in its very own .cpp and .hpp file. E.g. `FuncBreakable.cpp` and `FuncBreakable.hpp`. Having a .hpp file is not mandatory, but it's nice to have. 
  
## Dont's
 
Hungarian notation. Just no. If you don't know the type of a variable, just write it down on a notebook or something, or use an actual IDE.
  
Including headers into headers. NO. Use forward-declarations instead. There is an exception, and that is including the minimum of needed headers with one. For example: 
```
// File: PhysicsCore.hpp

#include "PhysLibrary/Common.hpp"
#include "PhysLibrary/Collision.hpp"
#include "PhysLibrary/Dynamics.hpp"
``` 
That is okay. Otherwise, avoid including headers into headers. 
 
*To-do: add more examples xwx*

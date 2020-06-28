# Code style guidelines

BurekTech's style is pretty straightforward.  
**NOTE:** *BurekTech is still currently 90% in C, as of June 2020, and as such, it follows id Software's style. Once a portion of it gets rewritten in C++, then these guidelines will apply.*
 
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
 

Pointers should "hug" the variable type instead of the variable name, as well as references:
```
BaseEntity* entity;
BaseEntity& ref = /*some entity, dunno*/;
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
 
Hungarian notation. (light sin)  
Just no. If you don't know the type of a variable, just write it down on a notebook or something, or use an actual IDE.
  
Including headers into headers. (heavy sin)  
NO. Use forward-declarations instead. There is an exception, and that is including the minimum of needed headers with one. For example: 
```
// File: PhysicsCore.hpp

#include "PhysLibrary/Common.hpp"
#include "PhysLibrary/Collision.hpp"
#include "PhysLibrary/Dynamics.hpp"
``` 
That is okay. Otherwise, avoid including headers into headers. 
 
*To-do: add more examples xwx*
 

## Conclusion

If you don't follow the rules and make light sins, your PR may still get accepted but in a form that follows the style guidelines. If you make heavy sins, then it will get rejected.

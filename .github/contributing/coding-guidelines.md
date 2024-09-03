# Scopy C++ coding guidelines

- Member variables should be prefixed with `m_`
- Getters and setters should be like in the following example:
```c++
bool m_enable;
bool enabled() const { return m_enable; }
void setEnabled(bool en) { m_enable = en; }
```
- `Variables` are `camelCase` (start with lowercase, and each word after that is uppercase)
- `Class names` always start with an `uppercase` letter.
- File names should be in full lowercase (header and source files)
- The class name should match the file name (case-insensitive)
- Always use explicit Qt macros (e.g. use `Q_SLOTS`, do not use slots)
- When naming classes, refrain from using Manager/Controller too much - https://stackoverflow.com/questions/1866794/naming-classes-how-to-avoid-calling-everything-a-whatevermanager and https://gist.github.com/hurricane-voronin/9ceccace0fd530bbf17c83b059c86eb7
- When implementing design patterns keep naming consistent so everyone is on the same page
  - Strategy pattern example - https://github.com/analogdevicesinc/scopy/tree/dev/plugins/regmap/src/readwrite
  - Singleton pattern example - https://github.com/analogdevicesinc/scopy/blob/dev/pluginbase/include/pluginbase/preferences.h
  - Factory pattern example - https://github.com/analogdevicesinc/scopy/blob/dev/core/include/core/devicefactory.h
    - Are factories singleton?
    - Or are they static classes?
    - Do they have QObject parents?
  - Resources for design patterns:
    - https://refactoring.guru/
    - https://www.youtube.com/watch?v=v9ejT8FO-7I&list=PLrhzvIcii6GNjpARdnO4ueTUAVR9eMBpc
    - Design Patterns: Elements of Reusable Object-Oriented Software Design patterns – Gang of Four
- Be aware of code smells - https://refactoring.guru/refactoring/smells , while not errors in the common sense, these "signs" suggest a deeper problem in design/architecture.

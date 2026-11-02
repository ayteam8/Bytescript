## Contributing to Bytescript

### Bug-fixes
  PRs concerning bug-fixes must clearly describe the problem as well as the solution  
  Preferably include the issue numbers relevant to the fix so they can be closed.

### New features / changes  
  Before starting to work on a new feature, you should first open an issue about the idea and wait for it to be approved. (not that you have to, but it might save you a few hours of work)  
  The change/feature should not have a too noticeable impact on the performance or memory usage of the language (unless of course it is an improvement) as this would go against one of the main goals of Bytescript.  
  The change/feature should have been tested to avoid edge-cases, bytescript (after the change) should successfully run both in a release build and a debug build (within a debugger) as it help avoiding issues.  
  The change/feature should not include any *Undefined Behavior* unless it is allowed by *gcc* (As some of Bytescript's feature rely on this compiler)  
  Please make sure your pull requests follow the [coding convention](#coding-conventions) below.  

### Coding conventions
  Coding conventions are mainly here so contributors don't get a headache reading the code.
  * Put spaces after comma separated items such as arrays (`{a, b, c}`) and function calls (`foo(a, b, c)`), as well as around operators (`x += a`).
  * One liners are limited to 2 statements
  * Camel-casing for function name as the repo is in C

### Use of AI
  To put simply, vibecoded commits will not be accepted.  
  The author of the PR should have written each line by hand in order to guarantee every function is fully understood  
  Prioritize trusted sources when looking for specific algorithms or programming language related topics, AI should only be used as last resort. (A well written article will deepen your understanding of the subject)

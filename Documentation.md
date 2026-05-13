***Matrix Library***

***Product Requirements***

***Performing matrix arithmetic by hand is slow and errors are possible, especially for matrices larger than 2×2. Students and developers can compile and do operations with matrices  in seconds. Every existing option either requires a heavy runtime (Python, MATLAB) or is too low-level to use directly.***

***Goals***

- ***Provide correct, clearly readable implementations of matrix addition, subtraction, and multiplication.***
- ***Allow matrices to be entered interactively or loaded from plain text  files.***
- ***Validate correctness against an established library (PyTorch).***
- ***Provide honest performance benchmarking .***

***Main Features***

***Dynamic matrix allocation  - Create matrices of any size at runtime***

***Three arithmetic operations - Addition, subtraction, multiplication***

` `***Interactive input mode - Enter matrix values by typing them in the terminal***

***File input/output mode  - Read matrices from text files, optionally save results***

***C benchmark - Measure raw multiplication speed at multiple sizes***

***PyTorch validation - Confirm C results match PyTorch to within floating point tolerance***

***PyTorch benchmark comparison - Side-by-side speed comparison  including GPU***

***Operation – Addition(`add`), Subtraction(`sub`), Multiplication(`mul`).***

***Error handling***

***Dimensions incompatible for the chosen operation - Prints a clear message explaining what was required.***

***File cannot be opened - Prints the filename that failed.***

***File has wrong format or too few - Prints which file caused the issue values.***                             

***NULL matrix passed to any function - No-op or returns NULL, never  crashes***

***Out-of-bounds `get`/`set` index - No-op or returns 0.0***

***Memory allocation failure - Returns NULL up the call stack***

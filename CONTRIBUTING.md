# Contributing to Orca-Light

First off, thank you for considering contributing to Orca-Light! It's people like you that make Orca-Light such a blazing fast, lightweight tool.

### 1. Where do I go from here?

If you've noticed a bug or have a feature request, make sure to check our Issues page to see if someone else in the community has already created a ticket. If not, go ahead and make one!

### 2. The Core Philosophy

When contributing code, please keep the core philosophy of Orca-Light in mind:
*   **Zero Bloat:** We do not use external heavy libraries. Native Win32 API is preferred.
*   **Maximum Performance:** Every CPU cycle matters. Ensure background threads run at `THREAD_PRIORITY_LOWEST` and yield (`Sleep(x)`) aggressively.
*   **Brutalist UI:** We do not use rounded corners, dropshadows, or fancy animations if they cost performance. Minimalist, monochrome, sharp.

### 3. Submitting a Pull Request

1. Fork the repo and create your branch from `main`.
2. If you've added code that should be tested, add tests.
3. If you've changed APIs, update the documentation.
4. Ensure the test suite passes.
5. Make sure your code lints and compiles with zero warnings under `x86_64-w64-mingw32-g++`.
6. Issue that pull request!

### Code Review Process
The core team looks at Pull Requests on a regular basis. After feedback has been given, we expect responses within two weeks. After that, we may close the pull request if it isn't showing any activity.

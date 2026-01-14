# Legacy .gitignore Template Notes

This document records the original usage notes for my older per‑project `.gitignore` template.  
It was designed for individual MSVS C++ console game projects and included rules to prevent accidental deletion of essential resource files.

Please carefully verify the properties and locations of required assets before applying `CleanProject.bat` or the `.gitignore` template.  
Incorrect rules may cause missing resources and confusing runtime errors.

## Basic Annotation Rules

### In `.gitignore`
Rule format:
<pattern>   # <reason>

Examples:
.obj            # Ignore compiler-generated object files
!Assets/.obj    # Preserve 3D model resources stored in Assets/

### In `CleanProject.bat`
Recommended:
:: Delete debug symbol files
del /s /q *.pdb

Not recommended:
del /s /q *.pdb   :: Delete debug symbol files

(Inline comments after commands may cause unexpected behavior in some environments.)
=============================================================================
    			Test exercise: Shell extension
=============================================================================
Solution for this task should be a COM component that is ready for 64-bit. Use MS Visual Studio 2008/2010/2012 (it can be Express Edition).

Main requirements:

Write COM component, that can be registered in MS Windows 7/8 64-bit. It should extend system shell to provide:
Extra menu item (example, "Calculate the Sum") in Windows Explorer’s context menu (right click on the set of selected files, folders should be ignored).
This menu item should write information about selected files into the log file.
Component should work on huge amount of selected files.
We prefer NOT to use ATL 

Nice to have:
 
Each line in the log file should represent: short filename, size in human readable form, creation date, etc.
Component should only append lines into the log file
Lines should be sorted by file names in alphabetical order.
In addition, it needs to calculate a per-byte sum (a flavour of checksum) in 4 bytes variable (DWORD) for each file.
The calculation of checksum should be done using concurrency.
Using a thread pool is a desirable solution.
Calculated information for each file should be logged as soon as possible, instead of at the end of processing of all files (sort order shall be preserved).

Additional requirements:

Source control systems (subversion, mercurial, git, etc.) are very useful for collaboration work. Use one of it to promote your solution (http://code.google.com, https://github.com, etc.).
C-style code is not welcome.
Use STL as much as possible.
Use C++11 or Boost for concurrency.
Implement your solution in cross-platform manner to be ready for use the business logic in Linux or Mac OS X application.
Cover your solution by unit tests (example, Google Test).
Test the business logic, not the COM specific code.
Get ready to describe all functions that you will use to implement the solution.

Hint:

To solve the main task you have to implement IShellExtInit and IContextMenu.
Essential COM is a good book to learn COM technology.


=============================================================================
    DYNAMIC LINK LIBRARY : CppShellExtContextMenuHandler Project Overview
=============================================================================

Summary:

The code sample demonstrates creating a Shell context menu handler with C++. 
A context menu handler is a shell extension handler that adds commands to an 
existing context menu.
Solution is based on Microsoft sample project
(http://code.msdn.microsoft.com/CppShellExtContextMenuHandl-410a709a).
License is included.

COM component was successfully tested on Windows 7 64bit.

Further Possible improvements:
- Code optimization
- Unit test coverage
- Fixing some bugs with mixed Russian/Ukranian symbols in filepaths.


/////////////////////////////////////////////////////////////////////////////
Setup and Removal:

A. Setup

If you are going to use the Shell extension in a x64 Windows system, please 
configure the Visual C++ project to target 64-bit platforms using project 
configurations (http://msdn.microsoft.com/en-us/library/9yb4317s.aspx). Only 
64-bit extension DLLs can be loaded in the 64-bit Windows Shell. 

In a command prompt running as administrator, navigate to the folder that 
contains the build result CppShellExtContextMenuHandler.dll and enter the 
command:

    Regsvr32.exe CppShellExtContextMenuHandler.dll

The context menu handler is registered successfully if you see a message box 
saying:

    "DllRegisterServer in CppShellExtContextMenuHandler.dll succeeded."

B. Removal

In a command prompt running as administrator, navigate to the folder that 
contains the build result CppShellExtContextMenuHandler.dll and enter the 
command:

    Regsvr32.exe /u CppShellExtContextMenuHandler.dll

The context menu handler is unregistered successfully if you see a message 
box saying:

    "DllUnregisterServer in CppShellExtContextMenuHandler.dll succeeded.


/////////////////////////////////////////////////////////////////////////////
Implementation:

A. Creating and configuring the project

In Visual Studio 2010, create a Visual C++ / Win32 / Win32 Project named 
"CppShellExtContextMenuHandler". In the "Application Settings" page of Win32 
Application Wizard, select the application type as "DLL" and check the "Empty 
project" option. After you click the Finish button, an empty Win32 DLL 
project is created.

-----------------------------------------------------------------------------

B. Implementing a basic Component Object Model (COM) DLL

Shell extension handlers are all in-process COM objects implemented as DLLs. 
Making a basic COM includes implementing DllGetClassObject, DllCanUnloadNow, 
DllRegisterServer, and DllUnregisterServer in (and exporting them from) the 
DLL, adding a COM class with the basic implementation of the IUnknown 
interface, preparing the class factory for your COM class. The relevant files 
in this code sample are:

  dllmain.cpp - implements DllMain and the DllGetClassObject, DllCanUnloadNow, 
    DllRegisterServer, DllUnregisterServer functions that are necessary for a 
    COM DLL. 

  GlobalExportFunctions.def - exports the DllGetClassObject, DllCanUnloadNow, 
    DllRegisterServer, DllUnregisterServer functions from the DLL through the 
    module-definition file. You need to pass the .def file to the linker by 
    configuring the Module Definition File property in the project's Property 
    Pages / Linker / Input property page.

  Reg.h/cpp - defines the reusable helper functions to register or unregister 
    in-process COM components in the registry: 
    RegisterInprocServer, UnregisterInprocServer

  FileContextMenuExt.h/cpp - defines the COM class. You can find the basic 
    implementation of the IUnknown interface in the files.

  ClassFactory.h/cpp - defines the class factory for the COM class. 

-----------------------------------------------------------------------------

C. Implementing the context menu handler 
-----------
Implementing the context menu handler:

The FileContextMenuExt.h/cpp files define a context menu handler. The 
context menu handler must implement the IShellExtInit and IContextMenu 
interfaces. A context menu extension is instantiated when the user displays 
the context menu.

    class FileContextMenuExt : public IShellExtInit, public IContextMenu
    {
    public:
        // IShellExtInit
        IFACEMETHODIMP Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT 
            pDataObj, HKEY hKeyProgID);

        // IContextMenu
        IFACEMETHODIMP QueryContextMenu(HMENU hMenu, UINT indexMenu, 
            UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
        IFACEMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO pici);
        IFACEMETHODIMP GetCommandString(UINT_PTR idCommand, UINT uFlags, 
            UINT *pwReserved, LPSTR pszName, UINT cchMax);
    };
	
  1. Implementing IShellExtInit

  After the context menu extension COM object is instantiated, the 
  IShellExtInit::Initialize method is called. IShellExtInit::Initialize 
  supplies the context menu extension with an IDataObject object that 
  holds one or more file names in CF_HDROP format. You can enumerate the 
  selected files and folders through the IDataObject object. If any value 
  other than S_OK is returned from IShellExtInit::Initialize, the context 
  menu extension will not be used.

  In the code sample, the FileContextMenuExt::Initialize method enumerates 
  the selected files and folders.

  2. Implementing IContextMenu

  After IShellExtInit::Initialize returns S_OK, the 
  IContextMenu::QueryContextMenu method is called to obtain the menu item or 
  items that the context menu extension will add. The QueryContextMenu 
  implementation is fairly straightforward. The context menu extension adds 
  its menu items using the InsertMenuItem or similar functions. The menu 
  command identifiers must be greater than or equal to idCmdFirst and must be 
  less than idCmdLast. QueryContextMenu must return the greatest numeric 
  identifier added to the menu plus one. The best way to assign menu command 
  identifiers is to start at zero and work up in sequence. If the context 
  menu extension does not need to add any items to the menu, it should simply 
  return zero from QueryContextMenu.

  In this code sample, the menu item "Calculate the Sum" was inserted with 
  an icon, and add a menu seperator below it.

  IContextMenu::GetCommandString is called to retrieve textual data for the 
  menu item, such as help text to be displayed for the menu item. If a user 
  highlights one of the items added by the context menu handler, the handler's 
  IContextMenu::GetCommandString method is called to request a Help text 
  string that will be displayed on the Windows Explorer status bar. This 
  method can also be called to request the verb string that is assigned to a 
  command. Either ANSI or Unicode verb strings can be requested. This example 
  only implements support for the Unicode values of uFlags, because only 
  those have been used in Windows Explorer since Windows 2000.

  IContextMenu::InvokeCommand is called when one of the menu items installed 
  by the context menu extension is selected. The context menu performs or 
  initiates the desired actions in response to this method.



/////////////////////////////////////////////////////////////////////////////
References:

MSDN: Initializing Shell Extensions
http://msdn.microsoft.com/en-us/library/cc144105.aspx

MSDN: Creating Context Menu Handlers
http://msdn.microsoft.com/en-us/library/bb776881.aspx

MSDN: Implementing the Context Menu COM Object
http://msdn.microsoft.com/en-us/library/ms677106.aspx

MSDN: Extending Shortcut Menus
http://msdn.microsoft.com/en-us/library/cc144101.aspx

MSDN: Choosing a Static or Dynamic Shortcut Menu Method
http://msdn.microsoft.com/en-us/library/dd758091.aspx

The Complete Idiot's Guide to Writing Shell Extensions
http://www.codeproject.com/KB/shell/shellextguide1.aspx
http://www.codeproject.com/KB/shell/shellextguide2.aspx
http://www.codeproject.com/KB/shell/shellextguide7.aspx

How to Use Submenus in a Context Menu Shell Extension
http://www.codeproject.com/KB/shell/ctxextsubmenu.aspx


/////////////////////////////////////////////////////////////////////////////
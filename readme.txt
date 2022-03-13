readme.txt
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FRequest v1.2a
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

----------------------------------
Description:
----------------------------------

FRequest is a fast, lightweight and opensource Windows / macOS / Linux desktop program to make HTTP(s) requests (e.g. call REST apis). 

The main motivation for the program is to create a similar working software to an IDE but for HTTP(s) apis. It should be fast, 
cross platform, lightweight, practical with a native look. Also it is important that project files can be 
easily shared and work seamless with Version Control System (VCS) for collaborative work.

The current features of FRequest are:

- Make GET / POST / PUT / DELETE / PATCH / HEAD / TRACE / OPTIONS HTTP(s) requests
- Make HTTP requests with RAW / Form Data or X-Form-WWW-UrlEncoded body types
- Send file uploads over the HTTP body type Form Data
- Analyze the requests response body and headers
- Requests are contained in a project, this project is then saved in XML file on user's desired location
- Ability to override a project main url, so you can make requests to different domain name addresses within the same project
- Ability to download files from the requests
- Automatically beautify and apply syntax highlighting for JSON and XML
- Support for authentication (HTTP Basic authentication and Request based authentication) which can be saved either in the
program configuration file (for private use) or the project file itself (for shared use)
- The FRequest project files are stored in a way which allow easy collaboration via a VCS like Git, Svn or Team Foundation Server
- Ability to add any kind of custom HTTP headers to the requests (automatically by taking the type in account or adding them 
manually)
- Global headers, headers that are applied to every request in the project
- Network proxy support

FRequest is licensed under GPL 3.0 (https://www.gnu.org/licenses/gpl-3.0.en.html).

----------------------------------
Supported operating system:
----------------------------------

- Windows 7 SP1 and above
- macOS High Sierra (10.13) and above
- Ubuntu 18.04 and above (other Linux distributions dated similar should likely work but are not tested)

----------------------------------
Installation:
----------------------------------

Windows / macOS:
Just extract the inner FRequest folder to any place in your computer. Run the executable.

Linux:
Extract the inner FRequest folder to any place in your computer, make it executable (chmod +x) and run the executable.

You may need to install openssl 1.1.1 to get the ssl requests to work properly
(and also to check for updates within the program).

In ubuntu (18.04) you can do it like this:
sudo apt-get install openssl

----------------------------------
Upgrading:
----------------------------------

Windows / macOS:
You should make a backup of your previous installation folder, just in case. After this
backup, extract the files from this zip version to your previous FRequest installation
folder (replace all files).

Linux:
You should make a backup of your previous installation folder, just in case. After this
backup, just replace the previous AppImage with the new one.

----------------------------------
Contacts:
----------------------------------

Author:
fabiobento512 (https://github.com/fabiobento512)

Official page:
https://fabiobento512.github.io/FRequest/

GitHub code page:
https://github.com/fabiobento512/FRequest

----------------------------------
Change Log:
----------------------------------
1.2a, 13-03-2022
- Upgraded Qt version on all platforms to 15.5.2, this fixes the openssl issue for linux users
(the problem that 1.0.1 is not the repositories anymore) and provides support for TLS 1.3
- Added github actions builds, which simplifies the building on the three supported operating systems a lot
- Minor code refactoring
----------------------------------
1.2, 18-05-2021
- Added global headers feature (thanks alevalv)
- Now request/reponse area is scrollabled (thanks fcolecumberri)
- Now request/reponse text uses courier new font (monospace) (thanks KaKi87 for the monospace suggestion)
----------------------------------
1.1c, 20-01-2019
- Added official support for Linux (tests are made in Ubuntu LTS and the program is distributed using an appimage)
- Fixed not showing body/headers response when an HTTP status code error is received like (500) (issue #7)
- Fixed timeout status code/message, when the internal FRequest timeout for a request is reached (issue #8)
- Added option in project settings to specify the type of character identation to use (space or tab) in the 
project xml file (issue #9)
----------------------------------
1.1b, 04-03-2018
- Added dark theme (thanks Jorgen-VikingGod!)
- Added a clear button for the filter text box
- Fixed possible system implementation of passwords obfuscation (you may need to re-enter your project passwords)
- Some code refactoring (using now override C++ keyword for instance)
----------------------------------
1.1a, 03-02-2018
- Added rename request / project to requests tree context menu (thanks pingzing!)
- Added shortcut to delete requests (DEL)
- Added an icon to delete request context menu
- Improved config/project files upgrade code
- Now it is possible to set the maximum response data size for display
- Fixed bug: on a new project, after saving the project properties 
the body data of the selected request may be cleared
- Now macOS users are warned about "App Translocation" when the application
can't create its .config file
- Added check for updates option in help menu
----------------------------------
1.1, 28-01-2018
- Morphed the QTextEdits to QPlainTextEdits in order to increase render performance for requests and responses data
- Now when the data is bigger than 200 kb only the first 200 kb are displayed in the interface, the remaining data is 
written to the disk if the request is marked to download (avoids slowdowns and possible crashes)
- Fixed bug where if the first request being loaded had overridden url the url didn't loaded
- Fixed bug where timeout 0 instead of mean no timeout, meant instant timeout
- Fixed bug where the first item loaded, if changed and then the project was saved, it wouldn't get properly saved
- Fixed bug where form-data were sending data in the format of x-www-form-urlencoded instead of form-data
- Now it is possible to cancel a running request
- Added icons for request types
- Some code refactoring
- Added icon to clone context menu
- Now when the project is selected in the requests tree the number of the requests for that project is displayed in 
the status bar
- Added support for file uploads
- Upgraded to Qt 5.10.0
- Now TRACE method should allow send of form and raw content
- Added support for XML highlighting
- Now cookies received by each request are saved by default (so they may be sent in next requests)
- Added requests filter so you can quickly find your requests
- Added authentication support (starting with Request authentication and HTTP Basic Authentication)
- Fixed the save of proxy settings
- Hide toolbar since it is unused
----------------------------------
1.0, 18-08-2017
- Initial Version
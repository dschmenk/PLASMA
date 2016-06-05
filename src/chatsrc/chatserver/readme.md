Mac OS X Errata
Apple's Python disables a function used by the chat server and Python client. To get a working version of Python, install the homebrew port manager (http://brew.sh) and updated Python with:

brew install python --with-poll

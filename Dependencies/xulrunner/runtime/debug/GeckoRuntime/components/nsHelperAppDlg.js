/*
//@line 42 "/c/mozilla/toolkit/mozapps/downloads/src/nsHelperAppDlg.js.in"
*/

/* This file implements the nsIHelperAppLauncherDialog interface.
 *
 * The implementation consists of a JavaScript "class" named nsUnknownContentTypeDialog,
 * comprised of:
 *   - a JS constructor function
 *   - a prototype providing all the interface methods and implementation stuff
 *
 * In addition, this file implements an nsIModule object that registers the
 * nsUnknownContentTypeDialog component.
 */


/* ctor
 */
function nsUnknownContentTypeDialog() {
    // Initialize data properties.
    this.mLauncher = null;
    this.mContext  = null;
    this.mSourcePath = null;
    this.chosenApp = null;
    this.givenDefaultApp = false;
    this.updateSelf = true;
    this.mTitle    = "";
}

nsUnknownContentTypeDialog.prototype = {
    nsIMIMEInfo  : Components.interfaces.nsIMIMEInfo,

    // This "class" supports nsIHelperAppLauncherDialog, and nsISupports.
    QueryInterface: function (iid) {
        if (!iid.equals(Components.interfaces.nsIHelperAppLauncherDialog) &&
            !iid.equals(Components.interfaces.nsISupports)) {
            throw Components.results.NS_ERROR_NO_INTERFACE;
        }
        return this;
    },

    // ---------- nsIHelperAppLauncherDialog methods ----------

    // show: Open XUL dialog using window watcher.  Since the dialog is not
    //       modal, it needs to be a top level window and the way to open
    //       one of those is via that route).
    show: function(aLauncher, aContext, aReason)  {
      this.mLauncher = aLauncher;
      this.mContext  = aContext;
      // Display the dialog using the Window Watcher interface.
      
      var ir = aContext.QueryInterface(Components.interfaces.nsIInterfaceRequestor);
      var dwi = ir.getInterface(Components.interfaces.nsIDOMWindowInternal);
      var ww = Components.classes["@mozilla.org/embedcomp/window-watcher;1"]
                .getService(Components.interfaces.nsIWindowWatcher);
      this.mDialog = ww.openWindow(dwi,
                                   "chrome://mozapps/content/downloads/unknownContentType.xul",
                                   null,
                                   "chrome,centerscreen,titlebar,dialog=yes,dependent",
                                   null);
      // Hook this object to the dialog.
      this.mDialog.dialog = this;
      
      // Hook up utility functions. 
      this.getSpecialFolderKey = this.mDialog.getSpecialFolderKey;
      
      // Watch for error notifications.
      this.progressListener.helperAppDlg = this;
      this.mLauncher.setWebProgressListener(this.progressListener);
    },

    // promptForSaveToFile:  Display file picker dialog and return selected file.
    //                       This is called by the External Helper App Service
    //                       after the ucth dialog calls |saveToDisk| with a null
    //                       target filename (no target, therefore user must pick).
    //
    //                       Alternatively, if the user has selected to have all
    //                       files download to a specific location, return that
    //                       location and don't ask via the dialog. 
    //
    // Note - this function is called without a dialog, so it cannot access any part
    // of the dialog XUL as other functions on this object do. 
    promptForSaveToFile: function(aLauncher, aContext, aDefaultFile, aSuggestedFileExtension) {
      var result = "";
      
      this.mLauncher = aLauncher;

      // If the user is always downloading to the same location, the default download
      // folder is stored in preferences. If a value is found stored, use that 
      // automatically and don't ask via a dialog. 
      var prefs = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);
      var autodownload = prefs.getBoolPref("browser.download.useDownloadDir");
      if (autodownload) {
        function getSpecialFolderKey(aFolderType) 
        {
          if (aFolderType == "Desktop")
            return "Desk";
        
          if (aFolderType != "Downloads")
            throw "ASSERTION FAILED: folder type should be 'Desktop' or 'Downloads'";
        
//@line 142 "/c/mozilla/toolkit/mozapps/downloads/src/nsHelperAppDlg.js.in"
          return "Pers";
//@line 150 "/c/mozilla/toolkit/mozapps/downloads/src/nsHelperAppDlg.js.in"
        }
        
        function getDownloadsFolder(aFolder)
        {
          var fileLocator = Components.classes["@mozilla.org/file/directory_service;1"].getService(Components.interfaces.nsIProperties);

          var dir = fileLocator.get(getSpecialFolderKey(aFolder), Components.interfaces.nsILocalFile);
          
          var bundle = Components.classes["@mozilla.org/intl/stringbundle;1"].getService(Components.interfaces.nsIStringBundleService);
          bundle = bundle.createBundle("chrome://mozapps/locale/downloads/unknownContentType.properties");

          var description = bundle.GetStringFromName("myDownloads");
          if (aFolder != "Desktop")
            dir.append(description);
            
          return dir;
        }

        var defaultFolder = null;
        switch (prefs.getIntPref("browser.download.folderList")) {
        case 0:
          defaultFolder = getDownloadsFolder("Desktop");
          break;
        case 1:
          defaultFolder = getDownloadsFolder("Downloads");
          break;
        case 2:
          defaultFolder = prefs.getComplexValue("browser.download.dir", Components.interfaces.nsILocalFile);
          break;
        }
        
        result = this.validateLeafName(defaultFolder, aDefaultFile, aSuggestedFileExtension);
      }
      
      if (!result) {
        // Use file picker to show dialog.
        var nsIFilePicker = Components.interfaces.nsIFilePicker;
        var picker = Components.classes["@mozilla.org/filepicker;1"].createInstance(nsIFilePicker);

        var bundle = Components.classes["@mozilla.org/intl/stringbundle;1"].getService(Components.interfaces.nsIStringBundleService);
        bundle = bundle.createBundle("chrome://mozapps/locale/downloads/unknownContentType.properties");

        var windowTitle = bundle.GetStringFromName("saveDialogTitle");
        var parent = aContext.QueryInterface(Components.interfaces.nsIInterfaceRequestor).getInterface(Components.interfaces.nsIDOMWindowInternal);
        picker.init(parent, windowTitle, nsIFilePicker.modeSave);
        picker.defaultString = aDefaultFile;

        if (aSuggestedFileExtension) {
          // aSuggestedFileExtension includes the period, so strip it
          picker.defaultExtension = aSuggestedFileExtension.substring(1);
        } 
        else {
          try {
            picker.defaultExtension = this.mLauncher.MIMEInfo.primaryExtension;
          } 
          catch (ex) { }
        }

        var wildCardExtension = "*";
        if (aSuggestedFileExtension) {
          wildCardExtension += aSuggestedFileExtension;
          picker.appendFilter(this.mLauncher.MIMEInfo.description, wildCardExtension);
        }

        picker.appendFilters( nsIFilePicker.filterAll );

        // Pull in the user's preferences and get the default download directory.
        var prefs = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);
        try {
          var startDir = prefs.getComplexValue("browser.download.dir", Components.interfaces.nsILocalFile);
          if (startDir.exists()) {
            picker.displayDirectory = startDir;
          }
        } 
        catch(exception) { }

        var dlgResult = picker.show();

        if (dlgResult == nsIFilePicker.returnCancel) {
          // null result means user cancelled.
          return null;
        }


        // Be sure to save the directory the user chose through the Save As... 
        // dialog  as the new browser.download.dir
        result = picker.file;

        if (result) {
          try {
            // Remove the file so that it's not there when we ensure non-existence later;
            // this is safe because for the file to exist, the user would have had to
            // confirm that he wanted the file overwritten.
            if (result.exists())
              result.remove(false);
          }
          catch (e) { }
          var newDir = result.parent;
          prefs.setComplexValue("browser.download.dir", Components.interfaces.nsILocalFile, newDir);
          result = this.validateLeafName(newDir, result.leafName, null);
        }
      }
      return result;
    },

    /**
     * Ensures that a local folder/file combination does not already exist in
     * the file system (or finds such a combination with a reasonably similar
     * leaf name), creates the corresponding file, and returns it.
     *
     * @param   aLocalFile
     *          the folder where the file resides
     * @param   aLeafName
     *          the string name of the file (may be empty if no name is known,
     *          in which case a name will be chosen)
     * @param   aFileExt
     *          the extension of the file, if one is known; this will be ignored
     *          if aLeafName is non-empty
     * @returns nsILocalFile
     *          the created file
     */
    validateLeafName: function (aLocalFile, aLeafName, aFileExt)
    {
      if (!aLocalFile || !aLocalFile.exists())
        return null;

      // Remove any leading periods, since we don't want to save hidden files
      // automatically.
      aLeafName = aLeafName.replace(/^\.+/, "");

      if (aLeafName == "")
        aLeafName = "unnamed" + (aFileExt ? "." + aFileExt : "");
      aLocalFile.append(aLeafName);

      this.makeFileUnique(aLocalFile);

      if (aLocalFile.isExecutable() && !this.mLauncher.targetFile.isExecutable()) {
        var f = aLocalFile.clone();
        aLocalFile.leafName = aLocalFile.leafName + "." + this.mLauncher.MIMEInfo.primaryExtension; 

        f.remove(false);
        this.makeFileUnique(aLocalFile);
      }

      return aLocalFile;
    },

    /**
     * Generates and returns a uniquely-named file from aLocalFile.  If
     * aLocalFile does not exist, it will be the file returned; otherwise, a
     * file whose name is similar to that of aLocalFile will be returned.
     */
    makeFileUnique: function (aLocalFile)
    {
      try {
        // Note - this code is identical to that in 
        //   toolkit/content/contentAreaUtils.js.
        // If you are updating this code, update that code too! We can't share code
        // here since this is called in a js component. 
        var collisionCount = 0;
        while (aLocalFile.exists()) {
          collisionCount++;
          if (collisionCount == 1) {
            // Append "(2)" before the last dot in (or at the end of) the filename
            // special case .ext.gz etc files so we don't wind up with .tar(2).gz
            if (aLocalFile.leafName.match(/\.[^\.]{1,3}\.(gz|bz2|Z)$/i)) {
              aLocalFile.leafName = aLocalFile.leafName.replace(/\.[^\.]{1,3}\.(gz|bz2|Z)$/i, "(2)$&");
            }
            else {
              aLocalFile.leafName = aLocalFile.leafName.replace(/(\.[^\.]*)?$/, "(2)$&");
            }
          }
          else {
            // replace the last (n) in the filename with (n+1)
            aLocalFile.leafName = aLocalFile.leafName.replace(/^(.*\()\d+\)/, "$1" + (collisionCount+1) + ")");
          }
        }
        aLocalFile.create(Components.interfaces.nsIFile.NORMAL_FILE_TYPE, 0600);
      }
      catch (e) {
        dump("*** exception in validateLeafName: " + e + "\n");
        if (aLocalFile.leafName == "" || aLocalFile.isDirectory()) {
          aLocalFile.append("unnamed");
          if (aLocalFile.exists())
            aLocalFile.createUnique(Components.interfaces.nsIFile.NORMAL_FILE_TYPE, 0600);
        }
      }
    },
    
    // ---------- implementation methods ----------

    // Web progress listener so we can detect errors while mLauncher is
    // streaming the data to a temporary file.
    progressListener: {
        // Implementation properties.
        helperAppDlg: null,

        // nsIWebProgressListener methods.
        // Look for error notifications and display alert to user.
        onStatusChange: function( aWebProgress, aRequest, aStatus, aMessage ) {
            if ( aStatus != Components.results.NS_OK ) {
                // Get prompt service.
                var prompter = Components.classes[ "@mozilla.org/embedcomp/prompt-service;1" ]
                                   .getService( Components.interfaces.nsIPromptService );
                // Display error alert (using text supplied by back-end).
                prompter.alert( this.dialog, this.helperAppDlg.mTitle, aMessage );

                // Close the dialog.
                this.helperAppDlg.onCancel();
                if ( this.helperAppDlg.mDialog ) {
                    this.helperAppDlg.mDialog.close();
                }
            }
        },

        // Ignore onProgressChange, onStateChange, onLocationChange, and onSecurityChange notifications.
        onProgressChange: function( aWebProgress,
                                    aRequest,
                                    aCurSelfProgress,
                                    aMaxSelfProgress,
                                    aCurTotalProgress,
                                    aMaxTotalProgress ) {
        },

        onProgressChange64: function( aWebProgress,
                                      aRequest,
                                      aCurSelfProgress,
                                      aMaxSelfProgress,
                                      aCurTotalProgress,
                                      aMaxTotalProgress ) {
        },



        onStateChange: function( aWebProgress, aRequest, aStateFlags, aStatus ) {
        },

        onLocationChange: function( aWebProgress, aRequest, aLocation ) {
        },

        onSecurityChange: function( aWebProgress, aRequest, state ) {
        }
    },

    // initDialog:  Fill various dialog fields with initial content.
    initDialog : function() {
      // Put file name in window title.
      var suggestedFileName = this.mLauncher.suggestedFileName;

      // Some URIs do not implement nsIURL, so we can't just QI.
      var url   = this.mLauncher.source;
      var fname = "";
      this.mSourcePath = url.prePath;
      try {
          url = url.QueryInterface( Components.interfaces.nsIURL );
          // A url, use file name from it.
          fname = url.fileName;
          this.mSourcePath += url.directory;
      } catch (ex) {
          // A generic uri, use path.
          fname = url.path;
          this.mSourcePath += url.path;
      }

      if (suggestedFileName)
        fname = suggestedFileName;
      
      var displayName = fname.replace(/ +/g, " ");

      this.mTitle = this.dialogElement("strings").getFormattedString("title", [displayName]);
      this.mDialog.document.title = this.mTitle;

      // Put content type, filename and location into intro.
      this.initIntro(url, fname, displayName);

      var iconString = "moz-icon://" + fname + "?size=16&contentType=" + this.mLauncher.MIMEInfo.MIMEType;
      this.dialogElement("contentTypeImage").setAttribute("src", iconString);

      // if always-save and is-executable and no-handler
      // then set up simple ui
      var mimeType = this.mLauncher.MIMEInfo.MIMEType;
      var shouldntRememberChoice = (mimeType == "application/octet-stream" || 
                                    mimeType == "application/x-msdownload" ||
                                    this.mLauncher.targetFile.isExecutable());
      if (shouldntRememberChoice && !this.openWithDefaultOK()) {
        // hide featured choice 
        this.mDialog.document.getElementById("normalBox").collapsed = true;
        // show basic choice 
        this.mDialog.document.getElementById("basicBox").collapsed = false;
        // change button labels
        this.mDialog.document.documentElement.getButton("accept").label = this.dialogElement("strings").getString("unknownAccept.label");
        this.mDialog.document.documentElement.getButton("cancel").label = this.dialogElement("strings").getString("unknownCancel.label");
        // hide other handler
        this.mDialog.document.getElementById("openHandler").collapsed = true;
        // set save as the selected option
        this.dialogElement("mode").selectedItem = this.dialogElement("save");
      }
      else {
        this.initAppAndSaveToDiskValues();

        // Initialize "always ask me" box. This should always be disabled
        // and set to true for the ambiguous type application/octet-stream.
        // We don't also check for application/x-msdownload here since we
        // want users to be able to autodownload .exe files. 
        var rememberChoice = this.dialogElement("rememberChoice");

//@line 476 "/c/mozilla/toolkit/mozapps/downloads/src/nsHelperAppDlg.js.in"
        if (shouldntRememberChoice) {
          rememberChoice.checked = false;
          rememberChoice.disabled = true;
        }
        else {
          rememberChoice.checked = !this.mLauncher.MIMEInfo.alwaysAskBeforeHandling;
        }
        this.toggleRememberChoice(rememberChoice);

        // XXXben - menulist won't init properly, hack. 
        var openHandler = this.dialogElement("openHandler");
        openHandler.parentNode.removeChild(openHandler);
        var openHandlerBox = this.dialogElement("openHandlerBox");
        openHandlerBox.appendChild(openHandler);
      }

      this.mDialog.setTimeout("dialog.postShowCallback()", 0);
      
      this.mDialog.document.documentElement.getButton("accept").disabled = true;
      const nsITimer = Components.interfaces.nsITimer;
      this._timer = Components.classes["@mozilla.org/timer;1"]
                              .createInstance(nsITimer);
      this._timer.initWithCallback(this, 250, nsITimer.TYPE_ONE_SHOT);
    },
    
    _timer: null,
    notify: function (aTimer) {
      try { // The user may have already canceled the dialog.
        if (!this._blurred)
          this.mDialog.document.documentElement.getButton('accept').disabled = false;
      } catch (ex) {}
      this._delayExpired = true;
      this._timer = null; // the timer won't release us, so we have to release it
    },
    
    postShowCallback: function () {
      this.mDialog.sizeToContent();

      // Set initial focus
      this.dialogElement("mode").focus();
    },

    // initIntro:
    initIntro: function(url, filename, displayname) {
        this.dialogElement( "location" ).value = displayname;
        this.dialogElement( "location" ).setAttribute("realname", filename);
        this.dialogElement( "location" ).setAttribute("tooltiptext", displayname);

        // if mSourcePath is a local file, then let's use the pretty path name instead of an ugly
        // url...
        var pathString = this.mSourcePath;
        try 
        {
          var fileURL = url.QueryInterface(Components.interfaces.nsIFileURL);
          if (fileURL)
          {
            var fileObject = fileURL.file;
            if (fileObject)
            {
              var parentObject = fileObject.parent;
              if (parentObject)
              {
                pathString = parentObject.path;
              }
            }
          }
        } catch(ex) {}

        if (pathString == this.mSourcePath)
        {
          // wasn't a fileURL
          var tmpurl = url.clone(); // don't want to change the real url
          try {
            tmpurl.userPass = "";
          } catch (ex) {}
          pathString = tmpurl.prePath;
        }

        // Set the location text, which is separate from the intro text so it can be cropped
        var location = this.dialogElement( "source" );
        location.value = pathString;
        location.setAttribute("tooltiptext", this.mSourcePath);
        
        // Show the type of file. 
        var type = this.dialogElement("type");
        var mimeInfo = this.mLauncher.MIMEInfo;
        
        // 1. Try to use the pretty description of the type, if one is available.
        var typeString = mimeInfo.description;
        
        if (typeString == "") {
          // 2. If there is none, use the extension to identify the file, e.g. "ZIP file"
          var primaryExtension = "";
          try {
            primaryExtension = mimeInfo.primaryExtension;
          }
          catch (ex) {
          }
          if (primaryExtension != "")
            typeString = primaryExtension.toUpperCase() + " file";
          // 3. If we can't even do that, just give up and show the MIME type. 
          else
            typeString = mimeInfo.MIMEType;
        }
        
        type.value = typeString;
    },
    
    _blurred: false,
    _delayExpired: false, 
    onBlur: function(aEvent) {
      if (aEvent.target != this.mDialog.document)
        return;
      this._blurred = true;
      this.mDialog.document.documentElement.getButton("accept").disabled = true;
    },
    
    onFocus: function(aEvent) {
      if (aEvent.target != this.mDialog.document)
        return;
      this._blurred = false;
      if (this._delayExpired) {
        var script = "document.documentElement.getButton('accept').disabled = false";
        this.mDialog.setTimeout(script, 250);
      }
    },

    // Returns true if opening the default application makes sense.
    openWithDefaultOK: function() {
        var result;

        // The checking is different on Windows...
//@line 609 "/c/mozilla/toolkit/mozapps/downloads/src/nsHelperAppDlg.js.in"
        // Windows presents some special cases.
        // We need to prevent use of "system default" when the file is
        // executable (so the user doesn't launch nasty programs downloaded
        // from the web), and, enable use of "system default" if it isn't
        // executable (because we will prompt the user for the default app
        // in that case).
        
        // Need to get temporary file and check for executable-ness.
        var tmpFile = this.mLauncher.targetFile;
        
        //  Default is Ok if the file isn't executable (and vice-versa).
        return !tmpFile.isExecutable();
//@line 627 "/c/mozilla/toolkit/mozapps/downloads/src/nsHelperAppDlg.js.in"
    },
    
    // Set "default" application description field.
    initDefaultApp: function() {
      // Use description, if we can get one.
      var desc = this.mLauncher.MIMEInfo.defaultDescription;
      if (desc) {
        var defaultApp = this.dialogElement("strings").getFormattedString("defaultApp", [desc]);
        this.dialogElement("defaultHandler").label = defaultApp;
      }
      else {
        this.dialogElement("modeDeck").setAttribute("selectedIndex", "1");
        // Hide the default handler item too, in case the user picks a 
        // custom handler at a later date which triggers the menulist to show.
        this.dialogElement("defaultHandler").hidden = true;
      }
    },

    // getPath:
    getPath: function (aFile) {
//@line 650 "/c/mozilla/toolkit/mozapps/downloads/src/nsHelperAppDlg.js.in"
      return aFile.path;
//@line 652 "/c/mozilla/toolkit/mozapps/downloads/src/nsHelperAppDlg.js.in"
    },

    // initAppAndSaveToDiskValues:
    initAppAndSaveToDiskValues: function() {
      var modeGroup = this.dialogElement("mode");

      // We don't let users open .exe files or random binary data directly 
      // from the browser at the moment because of security concerns. 
      var openWithDefaultOK = this.openWithDefaultOK();
      var mimeType = this.mLauncher.MIMEInfo.MIMEType;
      if (this.mLauncher.targetFile.isExecutable() || (
          (mimeType == "application/octet-stream" ||
           mimeType == "application/x-msdownload") && 
           !openWithDefaultOK)) {
        this.dialogElement("open").disabled = true;
        var openHandler = this.dialogElement("openHandler");
        openHandler.disabled = true;
        openHandler.selectedItem = null;
        modeGroup.selectedItem = this.dialogElement("save");
        return;
      }
    
      // Fill in helper app info, if there is any.
      this.chosenApp = this.mLauncher.MIMEInfo.preferredApplicationHandler;
      // Initialize "default application" field.
      this.initDefaultApp();

      var otherHandler = this.dialogElement("otherHandler");
              
      // Fill application name textbox.
      if (this.chosenApp && this.chosenApp.path) {
        otherHandler.setAttribute("path", this.getPath(this.chosenApp));
        otherHandler.label = this.chosenApp.leafName;
        otherHandler.hidden = false;
      }

      var useDefault = this.dialogElement("useSystemDefault");
      var openHandler = this.dialogElement("openHandler");
      openHandler.selectedIndex = 0;

      if (this.mLauncher.MIMEInfo.preferredAction == this.nsIMIMEInfo.useSystemDefault) {
        // Open (using system default).
        modeGroup.selectedItem = this.dialogElement("open");
      } else if (this.mLauncher.MIMEInfo.preferredAction == this.nsIMIMEInfo.useHelperApp) {
        // Open with given helper app.
        modeGroup.selectedItem = this.dialogElement("open");
        openHandler.selectedIndex = 1;
      } else {
        // Save to disk.
        modeGroup.selectedItem = this.dialogElement("save");
      }
      
      // If we don't have a "default app" then disable that choice.
      if (!openWithDefaultOK) {
        var useDefault = this.dialogElement("defaultHandler");
        var isSelected = useDefault.selected;
        
        // Disable that choice.
        useDefault.hidden = true;
        // If that's the default, then switch to "save to disk."
        if (isSelected) {
          openHandler.selectedIndex = 1;
          modeGroup.selectedItem = this.dialogElement("save");
        }
      }
      
      // otherHandler is always disabled on Mac
//@line 723 "/c/mozilla/toolkit/mozapps/downloads/src/nsHelperAppDlg.js.in"
      otherHandler.nextSibling.hidden = otherHandler.nextSibling.nextSibling.hidden = false;
//@line 725 "/c/mozilla/toolkit/mozapps/downloads/src/nsHelperAppDlg.js.in"
      this.updateOKButton();
    },

    // Returns the user-selected application
    helperAppChoice: function() {
      return this.chosenApp;
    },
    
    get saveToDisk() {
      return this.dialogElement("save").selected;
    },
    
    get useOtherHandler() {
      return this.dialogElement("open").selected && this.dialogElement("openHandler").selectedIndex == 1;
    },
    
    get useSystemDefault() {
      return this.dialogElement("open").selected && this.dialogElement("openHandler").selectedIndex == 0;
    },
    
    toggleRememberChoice: function (aCheckbox) {
        this.dialogElement("settingsChange").hidden = !aCheckbox.checked;
        this.mDialog.sizeToContent();
    },
    
    openHandlerCommand: function () {
      var openHandler = this.dialogElement("openHandler");
      if (openHandler.selectedItem.id == "choose")
        this.chooseApp();
      else
        openHandler.setAttribute("lastSelectedItemID", openHandler.selectedItem.id);
    },

    updateOKButton: function() {
      var ok = false;
      if (this.dialogElement("save").selected) {
        // This is always OK.
        ok = true;
      } 
      else if (this.dialogElement("open").selected) {
        switch (this.dialogElement("openHandler").selectedIndex) {
        case 0:
          // No app need be specified in this case.
          ok = true;
          break;
        case 1:
          // only enable the OK button if we have a default app to use or if 
          // the user chose an app....
          ok = this.chosenApp || /\S/.test(this.dialogElement("otherHandler").getAttribute("path")); 
        break;
        }
      }

      // Enable Ok button if ok to press.
      this.mDialog.document.documentElement.getButton("accept").disabled = !ok;
    },
    
    // Returns true iff the user-specified helper app has been modified.
    appChanged: function() {
      return this.helperAppChoice() != this.mLauncher.MIMEInfo.preferredApplicationHandler;
    },

    updateMIMEInfo: function() {
      var needUpdate = false;
      // If current selection differs from what's in the mime info object,
      // then we need to update.
      if (this.saveToDisk) {
        needUpdate = this.mLauncher.MIMEInfo.preferredAction != this.nsIMIMEInfo.saveToDisk;
        if (needUpdate)
          this.mLauncher.MIMEInfo.preferredAction = this.nsIMIMEInfo.saveToDisk;
      } 
      else if (this.useSystemDefault) {
        needUpdate = this.mLauncher.MIMEInfo.preferredAction != this.nsIMIMEInfo.useSystemDefault;
        if (needUpdate)
          this.mLauncher.MIMEInfo.preferredAction = this.nsIMIMEInfo.useSystemDefault;
      } 
      else {
        // For "open with", we need to check both preferred action and whether the user chose
        // a new app.
        needUpdate = this.mLauncher.MIMEInfo.preferredAction != this.nsIMIMEInfo.useHelperApp || this.appChanged();
        if (needUpdate) {
          this.mLauncher.MIMEInfo.preferredAction = this.nsIMIMEInfo.useHelperApp;
          // App may have changed - Update application and description
          var app = this.helperAppChoice();
          this.mLauncher.MIMEInfo.preferredApplicationHandler = app;
          this.mLauncher.MIMEInfo.applicationDescription = "";
        }
      }
      // We will also need to update if the "always ask" flag has changed.
      needUpdate = needUpdate || this.mLauncher.MIMEInfo.alwaysAskBeforeHandling != (!this.dialogElement("rememberChoice").checked);

      // One last special case: If the input "always ask" flag was false, then we always
      // update.  In that case we are displaying the helper app dialog for the first
      // time for this mime type and we need to store the user's action in the mimeTypes.rdf
      // data source (whether that action has changed or not; if it didn't change, then we need
      // to store the "always ask" flag so the helper app dialog will or won't display
      // next time, per the user's selection).
      needUpdate = needUpdate || !this.mLauncher.MIMEInfo.alwaysAskBeforeHandling;

      // Make sure mime info has updated setting for the "always ask" flag.
      this.mLauncher.MIMEInfo.alwaysAskBeforeHandling = !this.dialogElement("rememberChoice").checked;

      return needUpdate;        
    },
    
    // See if the user changed things, and if so, update the
    // mimeTypes.rdf entry for this mime type.
    updateHelperAppPref: function() {
      var ha = new this.mDialog.HelperApps();
      ha.updateTypeInfo(this.mLauncher.MIMEInfo);
      ha.destroy();
    },
    
    // onOK:
    onOK: function() {
      // Verify typed app path, if necessary.
      if (this.useOtherHandler) {
        var helperApp = this.helperAppChoice();
        if (!helperApp || !helperApp.exists()) {
          // Show alert and try again.        
          var bundle = this.dialogElement("strings");                    
          var msg = bundle.getFormattedString("badApp", [this.dialogElement("otherHandler").path]);
          var svc = Components.classes["@mozilla.org/embedcomp/prompt-service;1"].getService(Components.interfaces.nsIPromptService);
          svc.alert(this.mDialog, bundle.getString("badApp.title"), msg);

          // Disable the OK button.
          this.mDialog.document.documentElement.getButton("accept").disabled = true;
          this.dialogElement("mode").focus();          

          // Clear chosen application.
          this.chosenApp = null;

          // Leave dialog up.
          return false;
        }
      }
        
      // Remove our web progress listener (a progress dialog will be
      // taking over).
      this.mLauncher.setWebProgressListener(null);
      
      // saveToDisk and launchWithApplication can return errors in 
      // certain circumstances (e.g. The user clicks cancel in the
      // "Save to Disk" dialog. In those cases, we don't want to
      // update the helper application preferences in the RDF file.
      try {
        var needUpdate = this.updateMIMEInfo();
        
        if (this.dialogElement("save").selected) {
          // If we're using a default download location, create a path
          // for the file to be saved to to pass to |saveToDisk| - otherwise
          // we must ask the user to pick a save name.

//@line 892 "/c/mozilla/toolkit/mozapps/downloads/src/nsHelperAppDlg.js.in"
          this.mLauncher.saveToDisk(null, false);
        }
        else
          this.mLauncher.launchWithApplication(null, false);

        // Update user pref for this mime type (if necessary). We do not
        // store anything in the mime type preferences for the ambiguous
        // type application/octet-stream. We do NOT do this for 
        // application/x-msdownload since we want users to be able to 
        // autodownload these to disk. 
        if (needUpdate && this.mLauncher.MIMEInfo.MIMEType != "application/octet-stream")
          this.updateHelperAppPref();
      } catch(e) { }

      // Unhook dialog from this object.
      this.mDialog.dialog = null;

      // Close up dialog by returning true.
      return true;
    },

    // onCancel:
    onCancel: function() {
      // Remove our web progress listener.
      this.mLauncher.setWebProgressListener(null);

      // Cancel app launcher.
      try {
        const NS_BINDING_ABORTED = 0x804b0002;
        this.mLauncher.cancel(NS_BINDING_ABORTED);
      } catch(exception) {
      }

      // Unhook dialog from this object.
      this.mDialog.dialog = null;

      // Close up dialog by returning true.
      return true;
    },

    // dialogElement:  Convenience. 
    dialogElement: function(id) {
      return this.mDialog.document.getElementById(id);
    },

    // chooseApp:  Open file picker and prompt user for application.
    chooseApp: function() {
      var nsIFilePicker = Components.interfaces.nsIFilePicker;
      var fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(nsIFilePicker);
      fp.init(this.mDialog,
              this.dialogElement("strings").getString("chooseAppFilePickerTitle"),
              nsIFilePicker.modeOpen);

      fp.appendFilters(nsIFilePicker.filterApps);

      if (fp.show() == nsIFilePicker.returnOK && fp.file) {
        // Show the "handler" menulist since we have a (user-specified) 
        // application now.
        this.dialogElement("modeDeck").setAttribute("selectedIndex", "0");
        
        // Remember the file they chose to run.
        this.chosenApp = fp.file;
        // Update dialog.
        var otherHandler = this.dialogElement("otherHandler");
        otherHandler.removeAttribute("hidden");
        otherHandler.setAttribute("path", this.getPath(this.chosenApp));
        otherHandler.label = this.chosenApp.leafName;
        this.dialogElement("openHandler").selectedIndex = 1;
        this.dialogElement("openHandler").setAttribute("lastSelectedItemID", "otherHandler");
        
        this.dialogElement("mode").selectedItem = this.dialogElement("open");
      }
      else {
        var openHandler = this.dialogElement("openHandler");
        var lastSelectedID = openHandler.getAttribute("lastSelectedItemID");
        if (!lastSelectedID)
          lastSelectedID = "defaultHandler";
        openHandler.selectedItem = this.dialogElement(lastSelectedID);
      }
    },

    // Turn this on to get debugging messages.
    debug: false,

    // Dump text (if debug is on).
    dump: function( text ) {
        if ( this.debug ) {
            dump( text ); 
        }
    },

    // dumpInfo:
    doDebug: function() {
        const nsIProgressDialog = Components.interfaces.nsIProgressDialog;
        // Open new progress dialog.
        var progress = Components.classes[ "@mozilla.org/progressdialog;1" ]
                         .createInstance( nsIProgressDialog );
        // Show it.
        progress.open( this.mDialog );
    },

    // dumpObj:
    dumpObj: function( spec ) {
         var val = "<undefined>";
         try {
             val = eval( "this."+spec ).toString();
         } catch( exception ) {
         }
         this.dump( spec + "=" + val + "\n" );
    },

    // dumpObjectProperties
    dumpObjectProperties: function( desc, obj ) {
         for( prop in obj ) {
             this.dump( desc + "." + prop + "=" );
             var val = "<undefined>";
             try {
                 val = obj[ prop ];
             } catch ( exception ) {
             }
             this.dump( val + "\n" );
         }
    }
}

// This Component's module implementation.  All the code below is used to get this
// component registered and accessible via XPCOM.
var module = {
    firstTime: true,

    // registerSelf: Register this component.
    registerSelf: function (compMgr, fileSpec, location, type) {
        if (this.firstTime) {
            this.firstTime = false;
            throw Components.results.NS_ERROR_FACTORY_REGISTER_AGAIN;
        }
        compMgr = compMgr.QueryInterface(Components.interfaces.nsIComponentRegistrar);

        compMgr.registerFactoryLocation( this.cid,
                                         "Unknown Content Type Dialog",
                                         this.contractId,
                                         fileSpec,
                                         location,
                                         type );
    },

    // getClassObject: Return this component's factory object.
    getClassObject: function (compMgr, cid, iid) {
        if (!cid.equals(this.cid)) {
            throw Components.results.NS_ERROR_NO_INTERFACE;
        }

        if (!iid.equals(Components.interfaces.nsIFactory)) {
            throw Components.results.NS_ERROR_NOT_IMPLEMENTED;
        }

        return this.factory;
    },

    /* CID for this class */
    cid: Components.ID("{F68578EB-6EC2-4169-AE19-8C6243F0ABE1}"),

    /* Contract ID for this class */
    contractId: "@mozilla.org/helperapplauncherdialog;1",

    /* factory object */
    factory: {
        // createInstance: Return a new nsProgressDialog object.
        createInstance: function (outer, iid) {
            if (outer != null)
                throw Components.results.NS_ERROR_NO_AGGREGATION;

            return (new nsUnknownContentTypeDialog()).QueryInterface(iid);
        }
    },

    // canUnload: n/a (returns true)
    canUnload: function(compMgr) {
        return true;
    }
};

// NSGetModule: Return the nsIModule object.
function NSGetModule(compMgr, fileSpec) {
    return module;
}

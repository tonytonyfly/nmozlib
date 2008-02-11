@echo This will clean up all intermediate/user-specific files for NMozLib 
@echo ** You must close Visual C++ before running this file! **
@pause

@echo ========== Cleaning up... ==========
@del NMozLib.ncb
@del NMozLib.suo /AH
@del NMozLib\*.user
@rmdir NMozLib\Objects\Release\ /S /Q
@rmdir NMozLib\Objects\Debug\ /S /Q
@del NMozLib\Lib\*.lib
@del NMozLib\Lib\*.dll
@del NMozLib\Lib\*.ilk
@del NMozLib\Lib\*.exp
@del NMozLib\Lib\*.pdb
@echo ============== Done! ===============

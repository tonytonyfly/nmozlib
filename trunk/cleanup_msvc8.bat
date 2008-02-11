@echo This will clean up all intermediate/user-specific files for NMozLib 
@echo ** You must close Visual C++ before running this file! **
@pause

@echo ========== Cleaning up... ==========
@del NMozLib.ncb
@del NMozLib.suo /AH
@del NMozLib\*.user
@rmdir NMozLib\Objects\Release\ /S /Q
@rmdir NMozLib\Objects\Debug\ /S /Q
@del Navi\Lib\*.lib
@del Navi\Lib\*.dll
@del Navi\Lib\*.ilk
@del Navi\Lib\*.exp
@del Navi\Lib\*.pdb
@echo ============== Done! ===============

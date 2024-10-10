
@rem set "dbfile=.\output\invoicedb.db"
@rem set "siteoutdir=.\output\site"
@rem set "siteinstall=\\PESERVER\Groupdata\website\billing"

@rem mkdir output
@rem mkdir output\backup
@rem mkdir output\site

@rem dir /b /s /a:-d \\PESERVER\Groupdata\website\billing\FILES\ > shortlist.txt
@rem del core.db
type shortlist.txt | invoice-update-db



@rem dir /B /S /A:-D \\PESERVER\Groupdata\ScannedFiles\ScannedMaterial^(NEWSERVER2009^)\ | invoice-update-db -d "%dbfile%"
@rem dir /B /S /A:-D \\PESERVER\Groupdata\website\billing\FILES\ | invoice-update-db -d "%dbfile%"

@rem invoice-gen-site -d "%dbfile%" -o "%siteoutdir%"

@rem copy /f "%siteoutdir%\*.html" "%siteinstall%\"



set "dbfile=./output/invoicedb.db"
set "siteoutdir=./output/site"
set "siteinstall=//PESERVER/Groupdata/website/billing"

mkdir output
mkdir output/backup
mkdir output/site

dir /B /S \\PESERVER\Groupdata\ScannedFiles\ScannedMaterial^(NEWSERVER2009^)\ | invoice-update-db -d "%dbfile%"
dir /B /S \\PESERVER\Groupdata\website\billing\FILES\ | invoice-update-db -d "%dbfile%"

@rem invoice-gen-site -d "%dbfile%" -o "%siteoutdir%"

@rem copy /f "%siteoutdir%\*.html" "%siteinstall%\"


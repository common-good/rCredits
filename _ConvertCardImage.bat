REM For each Common Good Card PDF downloaded, convert and move it.
CD c:\documents\downloads
FOR %%f IN (*.cgcard.pdf) DO CALL:one %%f
GOTO:eof

REM Convert PDF to PNG, crop, and move it to card image directory
:one
REM 24% if original is 8" wide
SET qid=%~1
SET qid=%qid:.cgcard.pdf=%
MAGICK -density 600 "%~1" -resize 22.6%% -background white -alpha remove -crop 1145x725+0+0 +repage "c:\documents\CG\images\cgCards\%qid%.png"
DEL %~1
GOTO:eof

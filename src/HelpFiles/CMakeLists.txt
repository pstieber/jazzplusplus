# ---------------------------------------------------------------------------
# Jazz++ Help Files
# ---------------------------------------------------------------------------

# Install help images
install(
  FILES
    images/*.png
  DESTINATION
    share/Jazz++/HelpFiles/images
)

# Install help documents (pre-built HTML files)
install(
  FILES
    *.gif
    *.png
    *.html
    *.hhc
    *.hhk
    *.hhp
    *.htx
    *.con
    *.ref
  DESTINATION
    share/Jazz++/HelpFiles
)

helpimagesdir = ${prefix}/share/Jazz++/HelpFiles/images

make-install-dirs :
	-if test '!' -d $(helpimagesdir); then mkdir -p $(helpimagesdir); fi

install-data-hook : make-install-dirs
	-@ echo Installing $(helpimagesdir) ; \
	$(INSTALL_DATA) $(srcdir)/*.png $(helpimagesdir)

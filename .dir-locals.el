((nil . ((indent-tabs-mode . t)
	 (tab-width . 8)
	 (show-trailing-whitespace . t)
	 (whitespace-style face indentation)
	 (whitespace-line-column . 79)))
 (prog-mode .
	    ((c-file-style . "linux")
	     (eval ignore-errors
		   (require 'whitespace)
		   (whitespace-mode 1))))
 (cmake-mode .
	     ((cmake-tab-width . 8)
	      (eval ignore-errors
		    (require 'whitespace)
		    (whitespace-mode 1))))
 (python-mode .
	      ((indent-tabs-mode . nil)
	       (tab-width . 4)
	       (whitespace-line-column . 80)))
 )

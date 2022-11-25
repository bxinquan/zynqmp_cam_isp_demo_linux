if ("" != "") then
	if ($?DEBUGINFOD_URLS) then
		if ($%DEBUGINFOD_URLS) then
			setenv DEBUGINFOD_URLS "$DEBUGINFOD_URLS "
		else
			setenv DEBUGINFOD_URLS ""
		endif
	else
		setenv DEBUGINFOD_URLS ""
	endif
endif

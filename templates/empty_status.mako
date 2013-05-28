<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
 "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
    <title>Result summary</title>
    <link rel="stylesheet" href="status.css" type="text/css" />
  </head>
  <body>
    <h1>Result summary</h1>
    <p>Currently showing: ${page}</p>
    <p>Show:
      ## Index is a logical choice to put first, it will always be a link
      ## and we don't want in preceeded by a |
      <a href="index.html">index</a>
      % for i in pages:
        % if i == page:
          | ${i}
        % else:
          | <a href="${i}.html">${i}</a>
        % endif
      % endfor
    </p>
    <h1>No ${page}</h1>
  </body>
</html>

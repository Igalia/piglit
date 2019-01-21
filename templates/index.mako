<?xml version="1.0" encoding="UTF-8"?>

<%!
  import os
  import posixpath  # this must be posixpath, since we want /'s not \'s
  import re

  import six
  from six.moves import range

  from framework import grouptools, status

  def group_changes(test, current):
      group = grouptools.groupname(test)
      common = grouptools.commonprefix((current, group))

      common = grouptools.split(common)
      open = grouptools.split(group)[len(common):]
      close = grouptools.split(current)[len(common):]

      return open, close

  def group_result(result, group):
      """Get the worst status in a group."""
      if group not in result.totals:
          return status.NOTRUN

      return max([status.status_lookup(s) for s, v in
                  six.iteritems(result.totals[group]) if v > 0])

  def group_fraction(result, group):
      """Get the fraction value for a group."""
      if group not in result.totals:
          return '0/0'

      num = 0
      den = 0
      for k, v in six.iteritems(result.totals[group]):
          if v > 0:
              s = status.status_lookup(k)
              num += s.fraction[0] * v
              den += s.fraction[1] * v

      return '{}/{}'.format(num, den)


  def escape_filename(key):
      """Avoid reserved characters in filenames."""
      return re.sub(r'[<>:"|?*#]', '_', key)


  def escape_pathname(key):
      """ Remove / and \\ from names """
      return re.sub(r'[/\\]', '_', key)


  def normalize_href(href):
      """Force backward slashes in URLs."""
      return href.replace('\\', '/')
%>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
 "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
    <title>Result summary</title>
    <link rel="stylesheet" href="index.css" type="text/css" />
  </head>
  <body>
    <h1>Result summary</h1>
    <p>Currently showing: ${page}</p>
    <p>Show:
      % if page == 'all':
        all
      % else:
        <a href="index.html">all</a>
      % endif
      % for i in pages:
        % if i == page:
          | ${i}
        % else:
          | <a href="${i}.html">${i}</a>
        % endif
      % endfor
    </p>
    <table>
      <colgroup>
        ## Name Column
        <col />

        ## Status columns
        ## Create an additional column for each summary
        % for _ in range(len(results.results)):
        <col />
        % endfor
      </colgroup>
      <tr>
        <th/>
        % for res in results.results:
          <th class="head"><b>${res.name}</b><br />\
          (<a href="${normalize_href(os.path.join(escape_pathname(res.name), 'index.html'))}">info</a>)</th>
        % endfor
      </tr>
      <tr>
        <td class="head"><b>all</b></td>
        % for res in results.results:
          <td class="${group_result(res, 'root')}">
            <b>${group_fraction(res, 'root')}</b>
          </td>
        % endfor
      </tr>
      <%
        depth = 1
        group = ''
      %>
      % for test in sorted(getattr(results.names, page if page == 'all' else 'all_' + page)):
        <%
          open, close = group_changes(test, group)
          depth -= len(close)  # lower the indent for the groups we're not using
          if close:
            # remove the groups we're not using from current
            group = grouptools.split(group)[:-len(close)]
            if group:
              group = grouptools.join(*group)
            else:
              group = ''
        %>
        <tr>
        % if open:
          % for elem in open:
            <% group = grouptools.join(group, elem) %>
            ## Add the left most column, the name of the group
            <td>
              <div class="head" style="margin-left: ${depth * 1.75}em">
                <b>${elem | h}</b>
              </div>
            </td>
            ## add each group's totals
            % for res in results.results:
              <td class="${group_result(res, group)}">
                <b>${group_fraction(res, group)}</b>
              </td>
            % endfor
            <% depth += 1 %>
            </tr><tr>
          % endfor
        % endif
        
        <td>
          <div class="group" style="margin-left: ${depth * 1.75}em">
            ${grouptools.testname(test) | h}
          </div>
        </td>
        % for res in results.results:
          <%
            # Get the raw result, if it's none check to see if it's a subtest, if that's still None
            # then declare it not run
            # This very intentionally uses posix path, we're generating urls, and while
            # some windows based browsers support \\ as a url separator, *nix systems do not,
            # which would make a result generated on windows non-portable
            raw = res.tests.get(test)
            if raw is not None:
              result = raw.result
              href = normalize_href(posixpath.join(escape_pathname(res.name),
                                                   escape_filename(test)))
            else:
              raw = res.tests.get(grouptools.groupname(test))
              name = grouptools.testname(test)
              if raw is not None and name in raw.subtests:
                result = raw.subtests[name]
                href = normalize_href(posixpath.join(escape_pathname(res.name),
                                                     escape_filename(grouptools.groupname(test))))
              else:
                result = status.NOTRUN
            del raw  # we don't need this, so don't let it leak
          %>
          <td class="${str(result)}">
          % if str(result) not in exclude and result is not status.NOTRUN:
            <a href="${href}.html">
              ${str(result)}
            </a>
          % else:
            ${str(result)}
          % endif
          </td>
        % endfor
        </tr>
      % endfor
    </table>
  </body>
</html>

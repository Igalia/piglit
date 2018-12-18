![Build Status](https://gitlab.com/pages/sphinx/badges/master/build.svg)

---

Example [sphinx] documentation website using GitLab Pages.

Learn more about GitLab Pages at https://pages.gitlab.io and the official
documentation https://docs.gitlab.com/ce/user/project/pages/.

---

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [GitLab CI](#gitlab-ci)
- [Requirements](#requirements)
- [Building locally](#building-locally)
- [GitLab User or Group Pages](#gitlab-user-or-group-pages)
- [Did you fork this project?](#did-you-fork-this-project)
- [Troubleshooting](#troubleshooting)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## GitLab CI

This project's static Pages are built by [GitLab CI][ci], following the steps
defined in [`.gitlab-ci.yml`](.gitlab-ci.yml):

```
image: alpine

pages:
  script:
  - apk --no-cache add py2-pip python-dev
  - pip install sphinx
  - apk --no-cache add make
  - make html
  - mv _build/html/ public/
  artifacts:
    paths:
    - public
  only:
  - master
```

## Requirements

- [Sphinx][]

## Building locally

To work locally with this project, you'll have to follow the steps below:

1. Fork, clone or download this project
1. [Install][sphinx] Sphinx
1. Generate the documentation: `make`

The generated HTML will be located in the location specified by `conf.py`,
in this case `_build/html`.

## GitLab User or Group Pages

To use this project as your user/group website, you will need one additional
step: just rename your project to `namespace.gitlab.io`, where `namespace` is
your `username` or `groupname`. This can be done by navigating to your
project's **Settings**.

Read more about [user/group Pages][userpages] and [project Pages][projpages].

## Did you fork this project?

If you forked this project for your own use, please go to your project's
**Settings** and remove the forking relationship, which won't be necessary
unless you want to contribute back to the upstream project.

## Troubleshooting

No issues reported yet.

---

Forked from https://gitlab.com/Eothred/sphinx

[ci]: https://about.gitlab.com/gitlab-ci/
[userpages]: https://docs.gitlab.com/ce/user/project/pages/introduction.html#user-or-group-pages
[projpages]: https://docs.gitlab.com/ce/user/project/pages/introduction.html#project-pages
[sphinx]: http://www.sphinx-doc.org/

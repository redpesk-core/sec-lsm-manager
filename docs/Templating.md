# Templating

To create rules, sec-lsm-manager uses templates.
These templates are modified according to the name of the application and its permissions.

The project uses two tools to do that : m4 and mustach.

## m4

m4 is only used at compile time. It allows you to use a macro system that makes it easier
to write and understand rules.

The defined macros are:

| Macro                                       | Result                                                                             |
| ------------------------------------------- | ---------------------------------------------------------------------------------- |
| `MUSTACH_IF(xxx) ... [ ELSE ...] ENDIF`     | `{{#xxx}} ... [{{/xxx}}{{^xxx}} ...] {{/xxx}}`                                     |
| `MUSTACH_IF_NOT(xxx) ... [ ELSE ...] ENDIF` | `{{^xxx}} ... [{{/xxx}}{{#xxx}} ...] {{/xxx}}`                                     |
| `PERM(xxx)`                                 | `@PREFIX_PERMISSION@permission:xxx` where `@PREFIX_PERMISSION@` becomes `urn:AGL:` |
| `IF_PERM(xxx)`                              | is like `MUSTACH_IF(PERM(xxx))`                                                    |
| `IF_NOT_PERM(xxx)`                          | is like `MUSTACH_IF_NOT(PERM(xxx))`                                                |

For example, instead of writing :

```text
{{#urn:AGL:permission::partner:scope-platform}}
rw_platform_var({{id_underscore}}_t);
{{/urn:AGL:permission::partner:scope-platform}}
```

We write:

```text
MUSTACH_IF(urn:AGL:permission::partner:scope-platform)
rw_platform_var({{id_underscore}}_t);
ENDIF
```

Or because the permission starts with `urn:AGL:permission:`, it is enough to only write:

```text
IF_PERM(:partner:scope-platform)
rw_platform_var({{id_underscore}}_t);
ENDIF
```

## mustach

mustach allows to replace some fields at runtime and to verify conditions.

The marker `{{id}}` is replaced by the id of the installed application.

The marker `{{id_underscore}}` is replaced by the id of the installed application
and dashes (-) are replaced by underscores (_).

The markers `{{#xxx}}...{{/xxx}}` are replaced by `...` if the permission `xxx`
is granted or, otherwise, when permission is not granted, by nothing.

The markers `{{^xxx}}...{{/xxx}}` are the opposite, they are replaced by nothing
if the permission `xxx` is granted or, otherwise, when permission is not granted,
by `...`.

For example in our templates, for an application with the name `demo-app`, we will have the following replacements :

```text
{{id}} => demo-app
{{id_underscore}} => demo_app
```

And for the conditions the lines contained between our tags will be added only when
the permission has been granted.

For more information about mustach : [mustach-project](https://gitlab.com/jobol/mustach)

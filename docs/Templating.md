# Templating

To create rules, sec-lsm-manager uses templates.
These templates are modified according to the name of the application and its permissions.

The project uses two tools to do that : m4 and mustach.

## m4

m4 is only used at compile time. It allows you to use a macro system that makes it easier 
to write and understand rules.

The defined macros are:

- MUSTACH\_IF(xxx) ... [ ELSE ...] ENDIF:
  produces {{#xxx}} ... [{{/xxx}}{{^xxx}} ...] {{/xxx}}

- MUSTACH\_IF\_NOT(xxx) ... [ ELSE ...] ENDIF:
  produces {{^xxx}} ... [{{/xxx}}{{#xxx}} ...] {{/xxx}}

- PERM(xxx):
  produces @PREFIX\_PERMISSION@permission:xxx,
  where @PREFIX\_PERMISSION@ is substituted by `urn:AGL:`.

- IF\_PERM(xxx):
  is like MUSTACH\_IF(PERM(xxx))

- IF\_NOT\_PERM(xxx):
  is like MUSTACH\_IF\_NOT(PERM(xxx))

For example, instead of writing :

```
{{#urn:AGL:permission::partner:scope-platform}}
rw_platform_var({{id_underscore}}_t);
{{/urn:AGL:permission::partner:scope-platform}}
```

We write:

```
MUSTACH_IF(urn:AGL:permission::partner:scope-platform)
rw_platform_var({{id_underscore}}_t);
ENDIF
```

Or because the permission starts with `urn:AGL:permission:`, it is
enougth to only write:

```
IF_PERM(:partner:scope-platform)
rw_platform_var({{id_underscore}}_t);
ENDIF
```

## mustach

mustach allows to replace some fields at runtime and to verify conditions. 

The marker `{{id}}` is replaced by the id of the installed application.

The marker `{{id_underscore}}` is replaced by the id of the installed application
but where dashes (-) are replaced by underscores (_).

The markers `{{#xxx}}...{{/xxx}}` are replaced by `...` if the permission *xxx*
is granted or, otherwise, when permission is not granted, by nothing.

The markers `{{^xxx}}...{{/xxx}}` are the opposite, they are replaced by nothing
if the permission *xxx* is granted or, otherwise, when permission is not granted,
by `...`.

For example in our templates, for an application with the name `demo-app`, we will have the following replacements :

```
{{id}} => demo-app
{{id_underscore}} => demo_app
```

And for the conditions the lines contained between our tags will be added only when
the permission has been granted.

For more informations about mustach : [mustach-project](https://gitlab.com/jobol/mustach)


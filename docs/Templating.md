## Templating

To create rules, sec-lsm-manager uses templates.
These templates are modified according to the name of the application and its permissions.

The project uses two tools to do that : m4 and mustach.

### m4

m4 is only used at compile time. It allows you to use a macro system that makes it easier 
to write and understand rules.

For example, instead of writing :

```
{{#urn:AGL:permission::partner:scope-platform}}
rw_platform_var({{id_underscore}}_t);
{{/urn:AGL:permission::partner:scope-platform}}
```

We only write:

```
IF_PERM(:partner:scope-platform)
rw_platform_var({{id_underscore}}_t);
ENDIF
```

### mustach

mustach allows to replace some fields at runtime and to verify conditions. 
For example in our templates, for an application with the name `demo-app`, we will have the following replacements :

```
{{id}} => demo-app
{{id_underscore}} => demo_app
```

And for the conditions the lines contained between our tags will be added only when the permission has been granted.

For more informations about mustach : [mustach-project](https://gitlab.com/jobol/mustach)


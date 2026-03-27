; Scope/variable tracking queries for EZC.
; Used by nvim-treesitter for go-to-definition and reference highlighting.

; @name creates a binding in the current scope
(bind name: (identifier) @local.definition)

; $name is a reference to a binding
(recall name: (identifier) @local.reference)

; { ... } introduces a new local scope
(scope) @local.scope

; ( ... ) is a block — not a scope but captures bindings at call time
(block) @local.scope

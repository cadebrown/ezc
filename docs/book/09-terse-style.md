# Chapter 9: Terse Style

EZC rewards concise code, but short code still needs to be auditable.

Canonical stack words are 3 letters:

- `dup`, `del`, `swp`, `ovr`, `prt`

Symbol aliases:

- `,` -> `dup`
- `.` -> `del`
- `~` -> `swp`
- `_` -> `ovr`

## 9.1 Use Terseness In Layers

Recommended progression:

1. write canonical words while deriving logic
2. verify behavior
3. compress selective hot paths with aliases

## 9.2 Alias Examples

```ezc run
9 , * prt
```

```ezc run
1 2 ~ + prt
```

```ezc run
7 8 _ + + prt
```

## 9.3 Avoid Opaque Compression

This is short but still readable:

```ezc run
1 0 (dup prt 1 + dup 5 <) ^ del
```

If you cannot explain stack shape at each step, the program is too compressed.

## 9.4 Naming And Comments

Even concise EZC should carry intent:

- one-line header comments for each block
- state-shape comments in loops (`# state: n acc`)
- clear demo file names

## 9.5 Terse But Defensive

Add `del` deliberately to discard temporary values. Hidden leftovers are a common source of subtle bugs.

## Chapter Checkpoint

You should now have a style that is both terse and maintainable.

// fix-hostfiles: how it is built. Typst slides; build with "make" in this folder.
#import "@preview/touying:0.8.0": *
#import themes.metropolis: *
#import "@preview/fletcher:0.5.8" as fletcher: diagram, node, edge

#let guard = rgb("#B5432C")
#let sys = rgb("#2C6CB5")
#let ui = rgb("#3A8A4F")

#show: metropolis-theme.with(
  aspect-ratio: "16-9",
  footer: self => self.info.title,
  config-info(
    title: [fix-hostfiles],
    subtitle: [Unblocking a domain without breaking `/etc/hosts`, and how it is built],
    date: [October 2026],
  ),
)

#set text(size: 18pt)
#show raw: set text(font: "Menlo", size: 0.9em)

// Shared diagram styling: rounded boxes, filled arrows.
#let box(pos, body, color: black, thick: false, ..args) = node(
  pos, align(center, body),
  stroke: (paint: color, thickness: if thick { 2pt } else { 0.8pt }),
  corner-radius: 3pt, inset: 7pt, ..args,
)
#let flow(..args) = edge(..args, "-|>", stroke: 0.9pt)
#let no(body) = text(size: 0.65em, fill: guard, body)
#let small(body) = text(size: 0.8em, body)
#let chart(..args) = align(center, {
  set text(size: 17pt)
  diagram(label-size: 0.75em, ..args)
})

#title-slide()

== What fix-hostfiles is

- `hblock` maps ad, tracking and malware domains to `0.0.0.0` in `/etc/hosts`
- Sometimes it blocks a domain you need; fix-hostfiles is the way back
- *Four actions*, one at a time: `prep`, `restore`, `--add`, `--flush`
- *No shell*: every program it runs is started with `fork` and `execvp`
- *Atomic*: the hosts file is replaced with one `rename(2)`, never rewritten in place

#v(1.2em)
#small[About 1,100 lines of C11. The interesting part is the few dozen that
touch `/etc/hosts`.]

== Architecture

#chart(
  spacing: (14mm, 10mm),
  box((0, 0), [`main.c` \ #small[parse, dispatch]], color: ui, name: <main>),
  box((1, 0), [`fix-hosts.c` \ #small[the four actions]], name: <ops>),
  box((2, 0), [`system-actions.c` \ #small[files, processes, checks]], color: sys, name: <sys>),
  box((2, 1), [`/etc/hosts` \ `/etc/hblock/allow.list`], color: guard, thick: true, name: <etc>),
  box((1, 1), [`hblock`, `dscacheutil`, \ `killall`, `pgrep`], name: <ext>),
  flow(<main>, <ops>),
  flow(<ops>, <sys>),
  flow(<sys>, <etc>, [read, copy, rename], label-side: left),
  flow(<sys>, <ext>, [`execvp`], label-side: right),
)

#v(1.2em)
#small[`main.c` never touches a file; `fix-hosts.c` decides, `system-actions.c` does.]

== The command line: `main()`

#chart(
  spacing: (10mm, 5mm),
  box((0, 0), [`getopt_long`: `-h -v -f -a`], color: ui, name: <opt>),
  box((0, 1), [positional: `prep` or `restore`], color: ui, name: <pos>),
  box((0, 2), [exactly one action?], color: ui, name: <one>),
  box((1, 0), [`loadPaths`], name: <paths>),
  box((1, 1), [`switch (action)`], name: <sw>),
  box((2, 0), [`updateHostsFiles`], name: <u>),
  box((2, 1), [`addDnsName`], name: <a>),
  box((2, 2), [`flushDnsCache`], name: <f>),
  node((-1, 2), no[none or two: \ usage, exit 1], name: <bad>),
  flow(<opt>, <pos>), flow(<pos>, <one>),
  edge(<one>, <bad>, "-|>", stroke: guard),
  flow(<one>, (0.5, 2), (0.5, 0), <paths>),
  flow(<paths>, <sw>),
  flow(<sw>, <u>), flow(<sw>, <a>), flow(<sw>, <f>),
)

#v(1.2em)
- `selectAction` refuses a second action, so `--add x --flush` fails before anything runs
- `FIX_HOSTFILES_ETC_DIR` and `…_HBLOCK_DIR` move every path into a test fixture

== `prep` and `restore`: one function, two directions

#chart(
  spacing: (12mm, 6mm),
  box((0, 0), [source exists?], name: <s>),
  box((0, 1), [list `hosts*`], name: <l>),
  box((0, 2), [destination exists?], name: <d>),
  box((1, 2), [`confirm`: \ overwrite?], color: ui, name: <c>),
  box((2, 2), [`copyFile`], color: sys, name: <cp>),
  box((2, 1), [`prep` only: \ run `hblock`], color: sys, name: <h>),
  box((2, 0), [list `hosts*` again], name: <l2>),
  node((-1, 0), no[no: error], name: <e>),
  node((1, 1), no[no: “Exiting…”, exit 0], name: <x>),
  edge(<s>, <e>, "-|>", stroke: guard),
  flow(<s>, <l>), flow(<l>, <d>),
  flow(<d>, <c>, [yes]), flow(<c>, <cp>, [yes]),
  edge(<c>, <x>, "-|>", stroke: guard),
  flow(<d>, (0, 3), (2, 3), <cp>, [no], label-pos: 0.15),
  flow(<cp>, <h>), flow(<h>, <l2>),
)

#v(1.2em)
#align(center, table(
  columns: 3, stroke: none, inset: (x: 10pt, y: 4pt),
  table.hline(),
  [*Action*], [*Source*], [*Destination*],
  table.hline(stroke: 0.5pt),
  [`prep`], [`/etc/hosts`], [`/etc/hosts-ORIG`],
  [`restore`], [`/etc/hosts-ORIG`], [`/etc/hosts`],
  table.hline(),
))

== `--add`: unblocking one domain

#chart(
  spacing: (8mm, 7mm),
  box((0, 0), [`isValidDnsName` \ #small[POSIX regex]], color: guard, name: <v>),
  box((1, 0), [allow list has \ the exact line?], name: <has>),
  box((2, 0), [`appendLine`], color: sys, name: <app>),
  box((3, 0), [`removeHostEntry`], color: guard, thick: true, name: <rm>),
  node((0, 1), no[invalid: \ nothing written], name: <bad>),
  edge(<v>, <bad>, "-|>", stroke: guard),
  flow(<v>, <has>),
  flow(<has>, <app>, [no]),
  flow(<has>, (1, -0.7), (3, -0.7), <rm>, [yes: skip], label-pos: 0.5),
  flow(<app>, <rm>),
)

#v(1.2em)
- Adding twice leaves the allow list as adding once
- A hosts line goes only if a whitespace-separated token equals the name
  exactly: `0.0.0.0 ads.example.com` goes, `0.0.0.0 myads.example.com` stays
- Comments (`# …`) are never matched

== Atomic replacement: `removeHostEntry`

#chart(
  spacing: (6mm, 6mm),
  box((0, 0), [`copyFile` → \ `hosts.bak`], color: sys, name: <bak>),
  box((1, 0), [`mkstemp` \ `hosts.tmp.XXXXXX`], name: <tmp>),
  box((2, 0), [copy every line \ without the name], name: <copy>),
  box((3, 0), [`fflush`, `fsync`, \ `fchmod`, `fchown`], name: <sync>),
  box((4, 0), [`rename` over \ `/etc/hosts`], color: guard, thick: true, name: <mv>),
  flow(<bak>, <tmp>), flow(<tmp>, <copy>), flow(<copy>, <sync>), flow(<sync>, <mv>),
)

#v(1.2em)
#align(center, no[Any failure before `rename`: `unlink` the temporary file;
`/etc/hosts` is untouched.])

- The temporary file sits beside the hosts file, so `rename` stays on one filesystem
- A reader sees the whole old file or the whole new one, never half
- Owner and mode are copied from the original before the swap

== `--flush`: three programs, no shell

#chart(
  spacing: (10mm, 6mm),
  box((0, 0), [`uname`: \ Darwin?], name: <u>),
  box((1, 0), [`dscacheutil` \ `-flushcache`], color: sys, name: <a>),
  box((2, 0), [`killall -HUP` \ `mDNSResponder`], color: sys, name: <b>),
  box((3, 0), [`pgrep -fl` \ `mDNSResponder`], color: sys, name: <c>),
  node((0, 1), no[not macOS: refuse], name: <n>),
  node((3, 1), no[not found: warn, exit 1], name: <w>),
  edge(<u>, <n>, "-|>", stroke: guard),
  flow(<u>, <a>), flow(<a>, <b>, [4 s]), flow(<b>, <c>, [4 s]),
  edge(<c>, <w>, "-|>", stroke: guard),
)

#v(1.2em)
`runCommand` forks, the child calls `execvp(argv[0], argv)`, the parent
`waitpid`s and retries on `EINTR`. Arguments are an array, so a domain name
can never become shell syntax.

== The laws it keeps

Write $H$ for a hosts file, $A$ for the allow list, $d, e$ for DNS names.

#grid(
  columns: (1fr, 1fr), gutter: 1em,
  [
    *Removal idempotence* \
    $"remove"_d ("remove"_d (H)) = "remove"_d (H)$

    *Distinct removals commute* \
    $"remove"_d compose "remove"_e = "remove"_e compose "remove"_d$
  ],
  [
    *Insertion idempotence* \
    $"insert"_d ("insert"_d (A)) = "insert"_d (A)$

    *Prep–restore round trip* \
    $("restore" compose "prep")(H) = H$
  ],
)

#v(1.2em)
#small[Twelve laws in all, from `algebra/fix-hostfiles-algebra.tex`. The tests
check examples; the spec proposes property-based tests for these. The same formulas in
LaTeX and in Typst differ mainly in `\Remove{d}` versus `"remove"_d`.]

== Every call

#align(center)[
  #image("callgraph.svg", height: 82%)
  #let rows = csv("functions.csv", row-type: dictionary)
  #let get(k) = rows.find(r => r.file == k).functions
  #text(size: 0.6em)[Every call between fix-hostfiles' own functions:
  #get("total") functions, #get("calls") calls. Generated from clang's AST,
  not drawn.]
]

== Calls between files

#align(center, image("files.svg", width: 80%))

#v(1.2em)
#small[The same graph folded by file; numbers are distinct caller–callee pairs.
Nothing calls back into `main.c`, and `system-actions.c` calls nothing above it.]

== Where the functions are

#let roles = (
  "system-actions.c": [files, processes, validation, error reports],
  "fix-hosts.c": [paths and the four actions],
  "main.c": [argument parsing and dispatch],
)
#let rows = csv("functions.csv", row-type: dictionary).filter(r => r.file in roles)

#align(center, table(
  columns: 3, align: (left, right, left), stroke: none, inset: (x: 10pt, y: 6pt),
  table.hline(),
  [*File*], [*Functions*], [*Role*],
  table.hline(stroke: 0.5pt),
  ..rows.map(r => ([`src/`#raw(r.file)], r.functions, roles.at(r.file))).flatten(),
  table.hline(),
))

#v(1.2em)
#small[Read from `functions.csv` when the slides compile: Typst reads data
itself; the fsb Beamer slides had these numbers typed in.]

== By the numbers

#grid(
  columns: (1fr, 1fr), gutter: 2em,
  [
    - About 1,100 lines of C11
    - Nine end-to-end tests in Bash, run in a temporary fixture
    - Built with `-Wall -Wextra -Wpedantic -Werror`
  ],
  [
    - Started 25 February 2024
    - Algebraic spec in LaTeX: twelve laws
    - API docs with Doxygen, a man page
  ],
)

== How these slides were made

+ *Call graph* from clang's own parse:
  ```sh
  clang -fsyntax-only -Xclang -ast-dump=json src/main.c
  ```
+ *Filter*: a short Python script keeps calls between the program's own
  functions and writes Graphviz files and a CSV; `dot` draws SVGs
+ *Diagrams*: drawn by hand with the `fletcher` package, checked against the graph
+ *Slides*: Typst with `touying`, theme _metropolis_ (moloch's ancestor)

#v(1.2em)
#small[`make` in `docs/talk/` rebuilds everything from the current code;
`make watch` recompiles on every save in well under a second.]

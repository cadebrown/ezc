use dioxus::prelude::*;
use ezc::{
    build_pipeline,
    error::EzcError,
    ezcbc::{builtin_docs, Value},
    ezcvm::Vm,
    run_source,
};
use pulldown_cmark::{CodeBlockKind, Event, HeadingLevel, Options, Parser, Tag, TagEnd};
#[cfg(target_arch = "wasm32")]
use wasm_bindgen::prelude::*;

const DOCS_MD: &str = include_str!("../../docs/language.md");
const BOOK_CH01_MD: &str = include_str!("../../docs/book/01-stack-model.md");
const BOOK_CH02_MD: &str = include_str!("../../docs/book/02-values-and-literals.md");
const BOOK_CH03_MD: &str = include_str!("../../docs/book/03-stack-transforms.md");
const BOOK_CH04_MD: &str = include_str!("../../docs/book/04-branching-and-blocks.md");
const BOOK_CH05_MD: &str = include_str!("../../docs/book/05-loop-machines.md");
const BOOK_CH06_MD: &str = include_str!("../../docs/book/06-composite-data.md");
const BOOK_CH07_MD: &str = include_str!("../../docs/book/07-program-patterns.md");
const BOOK_CH08_MD: &str = include_str!("../../docs/book/08-tooling-and-debug.md");
const BOOK_CH09_MD: &str = include_str!("../../docs/book/09-terse-style.md");
const BOOK_CH10_MD: &str = include_str!("../../docs/book/10-practice-lab.md");
const DEMO_HELLO: &str = include_str!("../../demo/hello.ezc");
const DEMO_FIB: &str = include_str!("../../demo/fib.ezc");
const DEMO_GCD: &str = include_str!("../../demo/gcd.ezc");

const TERMINAL_ID: &str = "ezc-terminal";
const TERMINAL_BRIDGE_ID: &str = "ezc-terminal-bridge";
const XTERM_JS: Asset = asset!("/assets/vendor/xterm/xterm.js");
const XTERM_CSS: Asset = asset!("/assets/vendor/xterm/xterm.css");

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
struct BookChapterMeta {
    slug: &'static str,
    title: &'static str,
    summary: &'static str,
    markdown: &'static str,
}

// Ordered chapter registry used for index rendering and prev/next navigation.
const BOOK_CHAPTERS: &[BookChapterMeta] = &[
    BookChapterMeta {
        slug: "01-stack-model",
        title: "Chapter 1: The Stack Model",
        summary: "How to think in postfix and read stack transitions.",
        markdown: BOOK_CH01_MD,
    },
    BookChapterMeta {
        slug: "02-values-and-literals",
        title: "Chapter 2: Values And Literals",
        summary: "Numbers, text, symbols, and how literals become runtime values.",
        markdown: BOOK_CH02_MD,
    },
    BookChapterMeta {
        slug: "03-stack-transforms",
        title: "Chapter 3: Stack Transforms",
        summary: "Reordering with dup/del/swp/ovr and composing arithmetic pipelines.",
        markdown: BOOK_CH03_MD,
    },
    BookChapterMeta {
        slug: "04-branching-and-blocks",
        title: "Chapter 4: Branching And Blocks",
        summary: "Delayed blocks, explicit execution, and value-level branching with `?`.",
        markdown: BOOK_CH04_MD,
    },
    BookChapterMeta {
        slug: "05-loop-machines",
        title: "Chapter 5: Loop Machines",
        summary: "Build iterative state machines with `^` and truthy stop conditions.",
        markdown: BOOK_CH05_MD,
    },
    BookChapterMeta {
        slug: "06-composite-data",
        title: "Chapter 6: Composite Data",
        summary: "Nested stacks, symbolic data, and transporting structured values.",
        markdown: BOOK_CH06_MD,
    },
    BookChapterMeta {
        slug: "07-program-patterns",
        title: "Chapter 7: Program Patterns",
        summary: "Real program layouts from demos: fibonacci, gcd, factorial, and more.",
        markdown: BOOK_CH07_MD,
    },
    BookChapterMeta {
        slug: "08-tooling-and-debug",
        title: "Chapter 8: Tooling And Debugging",
        summary: "Use `check`, `disasm`, `--verbose`, and the web REPL to inspect behavior.",
        markdown: BOOK_CH08_MD,
    },
    BookChapterMeta {
        slug: "09-terse-style",
        title: "Chapter 9: Terse Style",
        summary: "Alias strategy, naming constraints, and concise but readable EZC style.",
        markdown: BOOK_CH09_MD,
    },
    BookChapterMeta {
        slug: "10-practice-lab",
        title: "Chapter 10: Practice Lab",
        summary: "Guided drills and capstone exercises to internalize EZC stack design.",
        markdown: BOOK_CH10_MD,
    },
];

const APP_CSS: &str = r#"
:root {
  --bg: #07090d;
  --bg-secondary: #0c1016;
  --panel: rgba(15, 20, 28, 0.92);
  --panel-strong: rgba(9, 13, 19, 0.95);
  --ink: #d1d7de;
  --muted: #7a8593;
  --rgb-r: #91656f;
  --rgb-g: #647c6f;
  --rgb-b: #677f95;
  --accent: #748395;
  --accent-strong: #8a9aab;
  --border: rgba(93, 110, 128, 0.5);
  --border-strong: rgba(116, 136, 157, 0.72);
  --shadow: 0 18px 50px rgba(0, 0, 0, 0.58);
}

* {
  box-sizing: border-box;
}

html,
body {
  margin: 0;
  min-height: 100%;
}

body {
  font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, "Liberation Mono",
    "Courier New", monospace;
  color: var(--ink);
  background:
    radial-gradient(circle at 15% 10%, rgba(100, 124, 111, 0.2), transparent 34%),
    radial-gradient(circle at 85% 0%, rgba(145, 101, 111, 0.16), transparent 36%),
    radial-gradient(circle at 55% 85%, rgba(103, 127, 149, 0.16), transparent 38%),
    linear-gradient(150deg, #05070b, #0d1219 45%, #070a0f);
}

a {
  color: inherit;
}

.shell {
  width: min(1120px, 95vw);
  margin: 0 auto;
  padding: 1.1rem 0 2.3rem;
}

.topbar {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 1rem;
  flex-wrap: wrap;
  margin-bottom: 0.95rem;
  padding: 0.62rem 0.85rem;
  border: 1px solid var(--border);
  border-radius: 2px;
  background: rgba(9, 13, 19, 0.88);
  backdrop-filter: blur(8px);
  box-shadow: inset 0 0 0 1px rgba(255, 255, 255, 0.02);
}

.brand {
  display: flex;
  gap: 0.7rem;
  align-items: baseline;
}

.brand strong {
  font-size: 1rem;
  letter-spacing: 0.08em;
  color: var(--accent-strong);
  text-transform: uppercase;
}

.brand span {
  color: var(--muted);
  font-size: 0.86rem;
}

.nav {
  display: flex;
  gap: 0.55rem;
  flex-wrap: wrap;
}

.nav-link {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  gap: 0.45rem;
  padding: 0.34rem 0.64rem;
  border-radius: 2px;
  border: 1px solid var(--border);
  background: rgba(255, 255, 255, 0.02);
  color: var(--muted);
  text-decoration: none;
  transition: 150ms ease;
}

.nav-link:hover {
  border-color: var(--border-strong);
  color: #c8d5e4;
}

.nav-link.active {
  color: var(--ink);
  border-color: rgba(103, 127, 149, 0.9);
  background: linear-gradient(
    140deg,
    rgba(103, 127, 149, 0.3),
    rgba(100, 124, 111, 0.28)
  );
}

.nav-icon {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  width: 1.15rem;
  height: 1.15rem;
  font-size: 0.72rem;
  border: 1px solid rgba(116, 136, 157, 0.6);
  color: #bccad8;
}

.hero {
  margin: 0.6rem 0 1rem;
  border: 1px solid var(--border);
  border-radius: 2px;
  padding: 0.9rem 1rem;
  background: linear-gradient(165deg, rgba(10, 15, 22, 0.95), rgba(13, 18, 27, 0.86));
  box-shadow: var(--shadow);
}

.hero h1 {
  margin: 0;
  font-size: clamp(1.8rem, 6.4vw, 3rem);
  letter-spacing: 0.03em;
  color: var(--ink);
  text-shadow:
    -1px 0 rgba(145, 101, 111, 0.35),
    1px 0 rgba(103, 127, 149, 0.35);
}

.hero p {
  margin: 0.35rem 0 0;
  color: var(--muted);
  max-width: 86ch;
}

.panel {
  background: linear-gradient(180deg, var(--panel), var(--panel-strong));
  border: 1px solid var(--border);
  border-radius: 2px;
  box-shadow: var(--shadow);
}

.content {
  padding: 1rem;
}

.grid {
  display: grid;
  gap: 0.85rem;
}

.grid.landing {
  grid-template-columns: repeat(auto-fit, minmax(230px, 1fr));
}

.card {
  padding: 0.92rem;
  border-left: 2px solid rgba(116, 136, 157, 0.45);
}

.card h3 {
  margin: 0 0 0.5rem;
  color: #bccbdb;
  font-size: 0.96rem;
}

.card p {
  margin: 0;
  color: var(--muted);
  font-size: 0.92rem;
  line-height: 1.5;
}

.card .goto {
  margin-top: 0.72rem;
  display: inline-flex;
  text-decoration: none;
  color: #a5b7ca;
}

.rule {
  margin: 1.2rem 0;
  height: 1px;
  background: linear-gradient(90deg, transparent, rgba(116, 136, 157, 0.64), transparent);
}

.doc {
  padding: 1rem;
}

.doc h1,
.doc h2,
.doc h3,
.doc h4 {
  margin: 1.1rem 0 0.58rem;
  color: #bccbdb;
}

.doc p {
  margin: 0.4rem 0 1rem;
  line-height: 1.62;
  color: #c8d0d9;
}

.doc pre {
  margin: 0.6rem 0 0.95rem;
  background: #080c12;
  border: 1px solid rgba(103, 127, 149, 0.52);
  border-radius: 2px;
  padding: 0.78rem;
  overflow-x: auto;
  color: #c6d3df;
}

.book-index-grid {
  display: grid;
  gap: 0.85rem;
  grid-template-columns: repeat(auto-fit, minmax(235px, 1fr));
}

.book-index-card h3 {
  margin: 0;
  color: #c8d4e0;
}

.book-index-card p {
  margin: 0.48rem 0 0;
}

.book-reader-layout {
  display: grid;
  gap: 0.85rem;
  grid-template-columns: minmax(230px, 300px) minmax(0, 1fr);
}

.book-sidebar {
  padding: 0.86rem;
}

.book-sidebar h3 {
  margin: 0;
  color: #c2cfdd;
}

.book-sidebar p {
  margin: 0.35rem 0 0.8rem;
  color: var(--muted);
  font-size: 0.88rem;
}

.book-chapter-list {
  display: grid;
  gap: 0.42rem;
}

.book-chapter-link {
  display: block;
  text-decoration: none;
  border: 1px solid rgba(116, 136, 157, 0.38);
  background: rgba(255, 255, 255, 0.01);
  color: #b4c0cc;
  padding: 0.5rem 0.55rem;
  transition: 120ms ease;
}

.book-chapter-link:hover {
  border-color: rgba(116, 136, 157, 0.76);
  color: #d4dce5;
}

.book-chapter-link.active {
  border-color: rgba(103, 127, 149, 0.9);
  background: linear-gradient(
    145deg,
    rgba(103, 127, 149, 0.28),
    rgba(100, 124, 111, 0.18)
  );
  color: #dde3ea;
}

.book-chapter-id {
  display: block;
  color: #8593a2;
  font-size: 0.76rem;
  margin-bottom: 0.15rem;
}

.book-nav-row {
  display: flex;
  flex-wrap: wrap;
  justify-content: space-between;
  gap: 0.55rem;
  margin-top: 1.25rem;
  padding-top: 0.82rem;
  border-top: 1px solid rgba(116, 136, 157, 0.45);
}

.book-nav-btn {
  text-decoration: none;
  border: 1px solid rgba(116, 136, 157, 0.68);
  color: #d2dae3;
  background: rgba(255, 255, 255, 0.02);
  padding: 0.34rem 0.6rem;
}

.book-nav-btn:hover {
  border-color: rgba(148, 166, 184, 0.92);
  color: #e4e9ee;
}

.snippet {
  margin: 0.95rem 0 1.2rem;
  border: 1px solid rgba(116, 136, 157, 0.44);
  border-radius: 2px;
  background: rgba(10, 15, 22, 0.94);
  padding: 0.85rem;
}

.snippet header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 0.7rem;
  flex-wrap: wrap;
  margin-bottom: 0.52rem;
}

.snippet-title {
  display: grid;
  gap: 0.2rem;
}

.snippet-title strong {
  color: #c2d0de;
}

.snippet-title span {
  color: var(--muted);
  font-size: 0.86rem;
}

button.run {
  border: 1px solid rgba(116, 136, 157, 0.7);
  background: linear-gradient(
    135deg,
    rgba(103, 127, 149, 0.24),
    rgba(100, 124, 111, 0.2)
  );
  color: #cfd8e2;
  font: inherit;
  padding: 0.35rem 0.72rem;
  border-radius: 2px;
  cursor: pointer;
}

button.run:hover {
  border-color: rgba(144, 163, 181, 0.9);
}

.output {
  margin-top: 0.62rem;
  border: 1px dashed rgba(145, 101, 111, 0.52);
  background: rgba(20, 13, 18, 0.88);
  border-radius: 2px;
  padding: 0.62rem;
  color: #d8c8cf;
  white-space: pre-wrap;
}

.terminal-layout {
  padding: 0.8rem;
}

.terminal-meta {
  margin: 0 0 0.7rem;
  color: var(--muted);
  font-size: 0.9rem;
}

.terminal-frame {
  height: min(72vh, 760px);
  min-height: 430px;
  border-radius: 2px;
  border: 1px solid rgba(103, 127, 149, 0.7);
  background: #070b11;
  box-shadow:
    0 0 0 1px rgba(145, 101, 111, 0.16),
    0 0 28px rgba(103, 127, 149, 0.16);
  overflow: hidden;
  position: relative;
}

.terminal-menu {
  position: absolute;
  left: 0.5rem;
  right: 0.5rem;
  top: 0.5rem;
  max-height: 45%;
  overflow: auto;
  background: rgba(10, 14, 20, 0.96);
  border: 1px solid rgba(116, 136, 157, 0.76);
  box-shadow: 0 10px 25px rgba(0, 0, 0, 0.55);
  color: #c9d2db;
  padding: 0.35rem 0.45rem;
  border-radius: 2px;
  z-index: 18;
  display: none;
  pointer-events: none;
  font-size: 0.8rem;
}

.terminal-menu.visible {
  display: block;
}

.terminal-menu .row {
  display: grid;
  grid-template-columns: minmax(3.4rem, 7.2rem) 1fr;
  gap: 0.55rem;
  align-items: baseline;
  padding: 0.1rem 0.2rem;
}

.terminal-menu .row.active {
  background: rgba(103, 127, 149, 0.24);
}

.terminal-menu .word {
  color: #d6dde5;
}

.terminal-menu .hint {
  color: #8f9cac;
  overflow-wrap: anywhere;
}

.terminal-menu .meta {
  margin-top: 0.25rem;
  border-top: 1px solid rgba(116, 136, 157, 0.45);
  padding-top: 0.2rem;
  color: #9eacbc;
}

.terminal-bridge {
  position: absolute;
  opacity: 0;
  pointer-events: none;
  width: 0;
  height: 0;
}

.notfound {
  padding: 1.3rem;
}

.notfound h2 {
  margin: 0 0 0.45rem;
  color: var(--rgb-r);
}

.notfound p {
  margin: 0;
  color: var(--muted);
}

@media (max-width: 760px) {
  .shell {
    width: min(1120px, 96vw);
    padding-top: 1rem;
  }

  .topbar {
    padding: 0.6rem 0.7rem;
  }

  .terminal-frame {
    min-height: 360px;
    height: 64vh;
  }

  .book-reader-layout {
    grid-template-columns: 1fr;
  }

  .book-nav-row {
    flex-direction: column;
    align-items: stretch;
  }
}
"#;

#[derive(Routable, Clone, Debug, PartialEq)]
enum Route {
    #[route("/")]
    Home {},
    #[route("/docs")]
    Docs {},
    #[route("/book")]
    Book {},
    #[route("/book/:slug")]
    BookChapter { slug: String },
    #[route("/repl")]
    Repl {},
    #[route("/:..segments")]
    NotFound { segments: Vec<String> },
}

#[derive(Clone, Debug, PartialEq)]
enum DocBlock {
    Heading {
        level: u8,
        text: String,
    },
    Paragraph(String),
    ListItem {
        ordered: bool,
        index: usize,
        text: String,
    },
    Code {
        language: String,
        code: String,
    },
    RunnableEzc(String),
    Rule,
}

#[derive(Clone, Debug, Default)]
struct WebReplState {
    vm: Vm,
}

enum TerminalReply {
    Print(String),
    Clear,
}

fn main() {
    launch();
}

pub fn launch() {
    dioxus::launch(App);
}

#[component]
fn App() -> Element {
    rsx! {
        style { "{APP_CSS}" }
        link { rel: "stylesheet", href: XTERM_CSS }
        script { src: XTERM_JS }
        Router::<Route> {}
    }
}

#[component]
fn Home() -> Element {
    rsx! {
        PageShell {
            title: "EZC Web",
            subtitle: "Static interface for EZC with REPL, docs, and learning book.",
            section {
                class: "grid landing",
                article {
                    class: "panel card",
                    h3 { "[T] /repl" }
                    p { "Terminal-style REPL with persistent stack state, history navigation, tab completion, and operator hints." }
                    Link { class: "goto", to: Route::Repl {}, "open ->" }
                }
                article {
                    class: "panel card",
                    h3 { "[D] /docs" }
                    p { "Language reference with runnable snippets and current operator semantics." }
                    Link { class: "goto", to: Route::Docs {}, "open ->" }
                }
                article {
                    class: "panel card",
                    h3 { "[B] /book" }
                    p { "Step-by-step guide for core EZC patterns and stack-flow reasoning." }
                    Link { class: "goto", to: Route::Book {}, "open ->" }
                }
            }

            div { class: "rule" }

            section {
                class: "panel content",
                h2 { "Quick sample runs" }
                p { style: "margin-top: 0.2rem; color: var(--muted);", "These execute in-browser using the EZC VM." }
                SampleSnippet {
                    title: "Hello, world",
                    description: "minimal text + print",
                    code: DEMO_HELLO.to_string(),
                }
                SampleSnippet {
                    title: "Fibonacci",
                    description: "iterative sequence builder with loop operator",
                    code: DEMO_FIB.to_string(),
                }
                SampleSnippet {
                    title: "GCD",
                    description: "euclidean algorithm with stack-friendly control flow",
                    code: DEMO_GCD.to_string(),
                }
            }
        }
    }
}

#[component]
fn Docs() -> Element {
    rsx! {
        PageShell {
            title: "EZC Docs",
            subtitle: "Reference syntax, operators, and semantics.",
            section {
                class: "panel doc",
                DocumentSection { markdown: DOCS_MD }
            }
        }
    }
}

#[component]
fn Book() -> Element {
    let chapter_cards = BOOK_CHAPTERS
        .iter()
        .enumerate()
        .map(|(idx, chapter)| (format!("[{:02}] {}", idx + 1, chapter.title), chapter))
        .collect::<Vec<_>>();

    rsx! {
        PageShell {
            title: "EZC Book",
            subtitle: "Walkthrough index. Open chapters in order and use prev/next navigation.",
            section {
                class: "book-index-grid",
                for (chapter_label, chapter) in chapter_cards {
                    article {
                        class: "panel card book-index-card",
                        h3 { "{chapter_label}" }
                        p { "{chapter.summary}" }
                        Link {
                            class: "goto",
                            to: Route::BookChapter {
                                slug: chapter.slug.to_string(),
                            },
                            "open chapter ->"
                        }
                    }
                }
            }
        }
    }
}

#[component]
fn BookChapter(slug: String) -> Element {
    let Some((current_idx, chapter)) = book_chapter_by_slug(&slug) else {
        return rsx! {
            PageShell {
                title: "Book Chapter Missing",
                subtitle: "That chapter slug was not found.",
                section {
                    class: "panel content",
                    p { "Unknown chapter: `{slug}`" }
                    p { "Available chapters:" }
                    div {
                        class: "book-chapter-list",
                        for chapter in BOOK_CHAPTERS {
                            Link {
                                class: "book-chapter-link",
                                to: Route::BookChapter {
                                    slug: chapter.slug.to_string(),
                                },
                                span { class: "book-chapter-id", "{chapter.slug}" }
                                "{chapter.title}"
                            }
                        }
                    }
                }
            }
        };
    };

    let previous = current_idx
        .checked_sub(1)
        .and_then(|index| BOOK_CHAPTERS.get(index));
    let next = BOOK_CHAPTERS.get(current_idx + 1);
    let sidebar_entries = BOOK_CHAPTERS
        .iter()
        .enumerate()
        .map(|(idx, item)| {
            let link_class = if idx == current_idx {
                "book-chapter-link active"
            } else {
                "book-chapter-link"
            };
            let id_label = format!("chapter {:02}", idx + 1);
            (link_class, id_label, item)
        })
        .collect::<Vec<_>>();

    rsx! {
        PageShell {
            title: chapter.title,
            subtitle: chapter.summary,
            section {
                class: "book-reader-layout",
                aside {
                    class: "panel book-sidebar",
                    h3 { "Chapter Index" }
                    p { "Read in order for a full walkthrough, or jump directly by topic." }
                    div {
                        class: "book-chapter-list",
                        for (link_class, id_label, item) in sidebar_entries {
                            Link {
                                class: "{link_class}",
                                to: Route::BookChapter {
                                    slug: item.slug.to_string(),
                                },
                                span { class: "book-chapter-id", "{id_label}" }
                                "{item.title}"
                            }
                        }
                    }
                }

                article {
                    class: "panel doc",
                    DocumentSection { markdown: chapter.markdown }
                    footer {
                        class: "book-nav-row",
                        if let Some(prev) = previous {
                            Link {
                                class: "book-nav-btn",
                                to: Route::BookChapter {
                                    slug: prev.slug.to_string(),
                                },
                                "← {prev.title}"
                            }
                        } else {
                            Link {
                                class: "book-nav-btn",
                                to: Route::Book {},
                                "← index"
                            }
                        }

                        Link {
                            class: "book-nav-btn",
                            to: Route::Book {},
                            "chapter index"
                        }

                        if let Some(next) = next {
                            Link {
                                class: "book-nav-btn",
                                to: Route::BookChapter {
                                    slug: next.slug.to_string(),
                                },
                                "{next.title} →"
                            }
                        } else {
                            Link {
                                class: "book-nav-btn",
                                to: Route::Book {},
                                "finished ✓"
                            }
                        }
                    }
                }
            }
        }
    }
}

fn book_chapter_by_slug(slug: &str) -> Option<(usize, &'static BookChapterMeta)> {
    BOOK_CHAPTERS
        .iter()
        .enumerate()
        .find(|(_, chapter)| chapter.slug == slug)
}

#[component]
fn Repl() -> Element {
    let mut booted = use_signal(|| false);
    let mut repl_state = use_signal(WebReplState::default);
    let completion_catalog = terminal_completion_catalog();

    use_effect(move || {
        if !*booted.read() {
            terminal_boot(TERMINAL_ID, TERMINAL_BRIDGE_ID);
            terminal_set_completions(TERMINAL_ID, &completion_catalog);
            booted.set(true);
        }
    });

    rsx! {
        PageShell {
            title: "EZC REPL",
            subtitle: "Terminal REPL with persistent stack and command/operator completion.",
            section {
                class: "panel terminal-layout",
                p {
                    class: "terminal-meta",
                    "Prompt: `∑`. Commands: `:help`, `:clear`, `:reset`. Up/Down navigates history. Tab opens a completion menu; Shift+Tab cycles backward."
                }
                div {
                    id: "{TERMINAL_ID}",
                    class: "terminal-frame"
                }
                input {
                    id: "{TERMINAL_BRIDGE_ID}",
                    class: "terminal-bridge",
                    r#type: "text",
                    oninput: move |evt| {
                        let source = evt.value();
                        let response = {
                            let mut state = repl_state.write();
                            execute_terminal_command(&mut state, &source)
                        };

                        match response {
                            TerminalReply::Print(text) => terminal_print(TERMINAL_ID, &text),
                            TerminalReply::Clear => terminal_clear(TERMINAL_ID),
                        }
                    },
                }
            }
        }
    }
}

#[component]
fn NotFound(segments: Vec<String>) -> Element {
    let attempted = if segments.is_empty() {
        "/".to_string()
    } else {
        format!("/{}", segments.join("/"))
    };

    rsx! {
        PageShell {
            title: "404",
            subtitle: "Route not found.",
            section {
                class: "panel notfound",
                h2 { "Unknown route: {attempted}" }
                p { "Try `/`, `/docs`, `/book`, `/book/<chapter-slug>`, or `/repl`." }
            }
        }
    }
}

#[component]
fn PageShell(title: &'static str, subtitle: &'static str, children: Element) -> Element {
    rsx! {
        main {
            class: "shell",
            header {
                class: "topbar",
                div {
                    class: "brand",
                    strong { "EZC [WEB]" }
                    span { "concatenative vm toolkit" }
                }
                nav {
                    class: "nav",
                    NavLink { to: Route::Home {}, icon: "H", label: "/" }
                    NavLink { to: Route::Repl {}, icon: "T", label: "/repl" }
                    NavLink { to: Route::Docs {}, icon: "D", label: "/docs" }
                    NavLink { to: Route::Book {}, icon: "B", label: "/book" }
                }
            }
            section {
                class: "hero",
                h1 { "{title}" }
                p { "{subtitle}" }
            }
            {children}
        }
    }
}

#[component]
fn NavLink(to: Route, icon: &'static str, label: &'static str) -> Element {
    let current = use_route::<Route>();
    let class = if route_matches_nav_target(&current, &to) {
        "nav-link active"
    } else {
        "nav-link"
    };

    rsx! {
        Link {
            class: "{class}",
            to,
            span { class: "nav-icon", "{icon}" }
            "{label}"
        }
    }
}

fn route_matches_nav_target(current: &Route, target: &Route) -> bool {
    match target {
        // Keep `/book` nav highlighted while browsing chapter sub-routes.
        Route::Book {} => matches!(current, Route::Book {} | Route::BookChapter { .. }),
        _ => current == target,
    }
}

#[component]
fn SampleSnippet(title: &'static str, description: &'static str, code: String) -> Element {
    let source = code.clone();
    let mut output = use_signal(|| "Click run to execute this sample".to_string());

    rsx! {
        article {
            class: "snippet",
            header {
                div {
                    class: "snippet-title",
                    strong { "{title}" }
                    span { "{description}" }
                }
                button {
                    class: "run",
                    onclick: move |_| output.set(run_snippet_output(&source)),
                    "▶ run"
                }
            }
            pre { code { "{code}" } }
            div {
                class: "output",
                "{output}"
            }
        }
    }
}

#[component]
fn DocumentSection(markdown: &'static str) -> Element {
    let blocks = parse_markdown(markdown);

    rsx! {
        for block in blocks {
            {render_block(block)}
        }
    }
}

fn render_block(block: DocBlock) -> Element {
    match block {
        DocBlock::Heading { level, text } => match level {
            1 => rsx! { h1 { "{text}" } },
            2 => rsx! { h2 { "{text}" } },
            3 => rsx! { h3 { "{text}" } },
            _ => rsx! { h4 { "{text}" } },
        },
        DocBlock::Paragraph(text) => {
            rsx! { p { "{text}" } }
        }
        DocBlock::ListItem {
            ordered,
            index,
            text,
        } => {
            let prefix = if ordered {
                format!("{index}. ")
            } else {
                "• ".to_string()
            };
            rsx! { p { "{prefix}{text}" } }
        }
        DocBlock::Code { language, code } => {
            let lang_label = if language.is_empty() {
                "text"
            } else {
                language.as_str()
            };
            rsx! {
                pre {
                    code { "[{lang_label}]\n{code}" }
                }
            }
        }
        DocBlock::RunnableEzc(code) => {
            rsx! {
                SampleSnippet {
                    title: "Runnable EZC snippet",
                    description: "from markdown",
                    code,
                }
            }
        }
        DocBlock::Rule => rsx! { div { class: "rule" } },
    }
}

fn run_snippet_output(source: &str) -> String {
    match run_source("<web-snippet>", source) {
        Ok(execution) => {
            if execution.stdout.is_empty() {
                format!("=> {}", render_stack(&execution.stack))
            } else if execution.stack.is_empty() {
                execution.stdout
            } else {
                format!(
                    "{}\n=> {}",
                    execution.stdout,
                    render_stack(&execution.stack)
                )
            }
        }
        Err(err) => summarize_error(&err).join("\n"),
    }
}

fn terminal_completion_catalog() -> String {
    let mut lines = vec![
        ":help\tshow REPL help".to_string(),
        ":clear\tclear terminal transcript".to_string(),
        ":reset\treset persistent stack".to_string(),
    ];

    for doc in builtin_docs() {
        let stack = format!(
            "stack {} -> {}",
            doc.stack_effect.before, doc.stack_effect.after
        );
        lines.push(format!("{}\t{} | {}", doc.canonical, doc.summary, stack));

        for alias in doc.aliases {
            lines.push(format!(
                "{alias}\talias of {} | {} | {}",
                doc.canonical, doc.summary, stack
            ));
        }
    }

    lines.join("\n")
}

fn execute_terminal_command(state: &mut WebReplState, source: &str) -> TerminalReply {
    let source = source.trim_end();

    if source.is_empty() {
        return TerminalReply::Print(String::new());
    }

    match source {
        ":help" => {
            return TerminalReply::Print(
                "EZC WEB REPL\n:help  show command help\n:clear clear terminal transcript\n:reset reset persistent stack\nEverything else is interpreted as EZC source."
                    .to_string(),
            );
        }
        ":clear" => return TerminalReply::Clear,
        ":reset" => {
            state.vm = Vm::default();
            return TerminalReply::Print("stack reset\n=> []".to_string());
        }
        _ => {}
    }

    let pipeline = match build_pipeline("<web-repl>", source) {
        Ok(pipeline) => pipeline,
        Err(err) => return TerminalReply::Print(summarize_error(&err).join("\n")),
    };

    let vm_snapshot = state.vm.clone();
    match state.vm.execute(&pipeline.bytecode).map_err(|err| {
        err.with_source_if_missing("<web-repl>", source)
            .with_debug("pipeline stage: vm")
    }) {
        Ok(result) => {
            let mut lines = Vec::new();
            if !result.stdout.is_empty() {
                lines.push(result.stdout);
            }
            lines.push(format!("=> {}", render_stack(&result.stack)));
            TerminalReply::Print(lines.join("\n"))
        }
        Err(err) => {
            state.vm = vm_snapshot;
            TerminalReply::Print(summarize_error(&err).join("\n"))
        }
    }
}

fn summarize_error(err: &EzcError) -> Vec<String> {
    let mut lines = Vec::new();
    lines.push(format!(
        "[{}@{}..{}] {}: {}",
        err.code.id(),
        err.span.start,
        err.span.end,
        err.code.title(),
        err.message
    ));

    for note in &err.notes {
        lines.push(format!("note: {note}"));
    }
    for help in &err.helps {
        lines.push(format!("help: {help}"));
    }
    for debug in &err.debug {
        lines.push(format!("debug: {debug}"));
    }

    lines
}

fn parse_markdown(input: &str) -> Vec<DocBlock> {
    #[derive(Clone, Copy)]
    struct ListState {
        ordered: bool,
        next_index: usize,
    }

    let parser = Parser::new_ext(input, Options::all());
    let mut blocks = Vec::new();

    let mut current_text = String::new();
    let mut heading_level: Option<u8> = None;
    let mut list_stack: Vec<ListState> = Vec::new();
    let mut in_item = false;

    let mut in_code = false;
    let mut code_language = String::new();
    let mut code_buffer = String::new();

    for event in parser {
        match event {
            Event::Start(Tag::Paragraph) => {
                if !in_item {
                    current_text.clear();
                }
            }
            Event::End(TagEnd::Paragraph) => {
                let text = current_text.trim();
                if !text.is_empty() {
                    if in_item {
                        current_text = text.to_string();
                    } else {
                        blocks.push(DocBlock::Paragraph(text.to_string()));
                        current_text.clear();
                    }
                } else if !in_item {
                    current_text.clear();
                }
            }
            Event::Start(Tag::Heading { level, .. }) => {
                heading_level = Some(heading_level_to_u8(level));
                current_text.clear();
            }
            Event::End(TagEnd::Heading(_)) => {
                let text = current_text.trim();
                if let Some(level) = heading_level.take() {
                    if !text.is_empty() {
                        blocks.push(DocBlock::Heading {
                            level,
                            text: text.to_string(),
                        });
                    }
                }
                current_text.clear();
            }
            Event::Start(Tag::CodeBlock(kind)) => {
                in_code = true;
                code_buffer.clear();
                code_language = match kind {
                    CodeBlockKind::Fenced(lang) => lang.into_string(),
                    CodeBlockKind::Indented => String::new(),
                };
            }
            Event::Start(Tag::List(start)) => {
                list_stack.push(ListState {
                    ordered: start.is_some(),
                    next_index: start.unwrap_or(1) as usize,
                });
            }
            Event::End(TagEnd::List(_)) => {
                list_stack.pop();
            }
            Event::Start(Tag::Item) => {
                in_item = true;
                current_text.clear();
            }
            Event::End(TagEnd::Item) => {
                let text = current_text.trim();
                if !text.is_empty() {
                    if let Some(list) = list_stack.last_mut() {
                        let index = list.next_index;
                        blocks.push(DocBlock::ListItem {
                            ordered: list.ordered,
                            index,
                            text: text.to_string(),
                        });
                        if list.ordered {
                            list.next_index += 1;
                        }
                    } else {
                        blocks.push(DocBlock::Paragraph(text.to_string()));
                    }
                }
                in_item = false;
                current_text.clear();
            }
            Event::End(TagEnd::CodeBlock) => {
                in_code = false;
                let code = code_buffer.trim_end().to_string();
                if is_ezc_runnable(&code_language) {
                    blocks.push(DocBlock::RunnableEzc(code));
                } else {
                    blocks.push(DocBlock::Code {
                        language: code_language.clone(),
                        code,
                    });
                }
                code_buffer.clear();
                code_language.clear();
            }
            Event::Text(text) => {
                if in_code {
                    code_buffer.push_str(&text);
                } else {
                    current_text.push_str(&text);
                }
            }
            Event::Code(code) => {
                current_text.push('`');
                current_text.push_str(&code);
                current_text.push('`');
            }
            Event::SoftBreak | Event::HardBreak => {
                if in_code {
                    code_buffer.push('\n');
                } else {
                    current_text.push('\n');
                }
            }
            Event::Rule => blocks.push(DocBlock::Rule),
            _ => {}
        }
    }

    blocks
}

fn heading_level_to_u8(level: HeadingLevel) -> u8 {
    match level {
        HeadingLevel::H1 => 1,
        HeadingLevel::H2 => 2,
        HeadingLevel::H3 => 3,
        HeadingLevel::H4 => 4,
        HeadingLevel::H5 => 5,
        HeadingLevel::H6 => 6,
    }
}

fn is_ezc_runnable(language: &str) -> bool {
    let normalized = language.trim().to_ascii_lowercase();
    normalized == "ezc run" || normalized == "ezc-run" || normalized == "ezc"
}

fn render_stack(stack: &[Value]) -> String {
    let inner = stack
        .iter()
        .map(Value::to_source)
        .collect::<Vec<_>>()
        .join(" ");
    format!("[{inner}]")
}

#[cfg(target_arch = "wasm32")]
#[wasm_bindgen(inline_js = r##"
const __ezcTermRegistry = globalThis.__ezcTermRegistry || (globalThis.__ezcTermRegistry = {});
const __ezcTermPendingCompletions =
  globalThis.__ezcTermPendingCompletions || (globalThis.__ezcTermPendingCompletions = {});

function __ezcPrintable(ch) {
  const code = ch.charCodeAt(0);
  return code >= 0x20 && code !== 0x7f;
}

function __ezcParseCompletions(raw) {
  const text = String(raw ?? "");
  return text
    .split("\n")
    .map((line) => line.trimEnd())
    .filter((line) => line.length > 0)
    .map((line) => {
      const idx = line.indexOf("\t");
      if (idx < 0) {
        return { word: line.trim(), hint: "" };
      }
      return {
        word: line.slice(0, idx).trim(),
        hint: line.slice(idx + 1).trim(),
      };
    })
    .filter((entry) => entry.word.length > 0);
}

function __ezcTokenAtEnd(input) {
  let start = input.length;
  while (start > 0) {
    const ch = input[start - 1];
    if (ch === " " || ch === "\t" || ch === "\n" || ch === "\r") {
      break;
    }
    start -= 1;
  }
  return { start, prefix: input.slice(start) };
}

function __ezcEnsureMenu(state) {
  if (state.menuEl) {
    return state.menuEl;
  }
  const menu = document.createElement("div");
  menu.className = "terminal-menu";
  state.host.appendChild(menu);
  state.menuEl = menu;
  return menu;
}

function __ezcHideMenu(state) {
  if (state.menuHideTimer) {
    clearTimeout(state.menuHideTimer);
    state.menuHideTimer = null;
  }
  if (state.menuEl) {
    state.menuEl.classList.remove("visible");
    state.menuEl.innerHTML = "";
  }
}

function __ezcFlashMenu(state, message) {
  const menu = __ezcEnsureMenu(state);
  menu.innerHTML = "";
  const line = document.createElement("div");
  line.className = "meta";
  line.textContent = message;
  menu.appendChild(line);
  menu.classList.add("visible");

  if (state.menuHideTimer) {
    clearTimeout(state.menuHideTimer);
  }
  state.menuHideTimer = setTimeout(() => {
    __ezcHideMenu(state);
  }, 900);
}

function __ezcRenderMenu(state) {
  if (!state.completion || !state.completion.matches.length) {
    __ezcHideMenu(state);
    return;
  }

  const menu = __ezcEnsureMenu(state);
  menu.innerHTML = "";

  const matches = state.completion.matches;
  const index = state.completion.index;
  const maxRows = 8;
  const start =
    matches.length > maxRows
      ? Math.max(
          0,
          Math.min(index - Math.floor(maxRows / 2), matches.length - maxRows)
        )
      : 0;
  const end = Math.min(start + maxRows, matches.length);

  for (let i = start; i < end; i += 1) {
    const entry = matches[i];
    const row = document.createElement("div");
    row.className = "row" + (i === index ? " active" : "");

    const word = document.createElement("span");
    word.className = "word";
    word.textContent = entry.word;

    const hint = document.createElement("span");
    hint.className = "hint";
    hint.textContent = entry.hint;

    row.appendChild(word);
    row.appendChild(hint);
    menu.appendChild(row);
  }

  const meta = document.createElement("div");
  meta.className = "meta";
  meta.textContent = `${index + 1}/${matches.length}  Tab: cycle  Shift+Tab: reverse`;
  menu.appendChild(meta);
  menu.classList.add("visible");
}

function __ezcResize(state) {
  const width = Math.max(40, Math.floor(state.host.clientWidth / 9));
  const height = Math.max(16, Math.floor(state.host.clientHeight / 18));
  try {
    state.term.resize(width, height);
  } catch (_) {
    // ignore while terminal is initializing
  }
}

function __ezcSetInput(state, next) {
  state.input = next;
  state.term.write("\r\x1b[2K" + state.prompt + state.input);
}

export function ezcTerminalBoot(containerId, bridgeId) {
  if (__ezcTermRegistry[containerId]) {
    return;
  }

  const bootstrap = () => {
    if (!globalThis.Terminal) {
      setTimeout(bootstrap, 24);
      return;
    }

    const host = document.getElementById(containerId);
    const bridge = document.getElementById(bridgeId);
    if (!host || !bridge) {
      setTimeout(bootstrap, 24);
      return;
    }

    if (__ezcTermRegistry[containerId]) {
      return;
    }

    const term = new globalThis.Terminal({
      cursorBlink: true,
      theme: {
        background: "#070b11",
        foreground: "#d1d7de",
        cursor: "#8a9aab",
        black: "#070b11",
        red: "#91656f",
        green: "#647c6f",
        yellow: "#8a8071",
        blue: "#677f95",
        magenta: "#7f728f",
        cyan: "#6f8797",
        white: "#cfd5dc",
        brightBlack: "#2e3640",
        brightRed: "#aa7a84",
        brightGreen: "#789183",
        brightYellow: "#9b9182",
        brightBlue: "#7b93aa",
        brightMagenta: "#9688a7",
        brightCyan: "#839ba9",
        brightWhite: "#e5e8eb",
      },
      fontFamily: "ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, Liberation Mono, Courier New, monospace",
      fontSize: 15,
      lineHeight: 1.25,
      scrollback: 4000,
      convertEol: true,
    });

    term.open(host);

    const state = {
      host,
      bridge,
      term,
      prompt: "∑ ",
      input: "",
      history: [],
      historyIndex: 0,
      locked: false,
      completions: __ezcParseCompletions(__ezcTermPendingCompletions[containerId]),
      completion: null,
      menuEl: null,
      menuHideTimer: null,
    };

    __ezcTermRegistry[containerId] = state;

    const submitLine = () => {
      const line = state.input;
      state.input = "";
      state.completion = null;
      __ezcHideMenu(state);
      state.term.write("\r\n");

      if (line.trim().length > 0) {
        state.history.push(line);
      }
      state.historyIndex = state.history.length;

      if (line.trim().length === 0) {
        state.locked = false;
        state.term.write(state.prompt);
        return;
      }

      state.locked = true;
      state.bridge.value = line;
      state.bridge.dispatchEvent(new Event("input", { bubbles: true }));
    };

    term.onData((data) => {
      if (state.locked) {
        return;
      }

      if (data === "\r") {
        if (state.completion && state.completion.matches.length > 0) {
          const selected = state.completion.matches[state.completion.index];
          const token = __ezcTokenAtEnd(state.input);
          const start =
            state.completion.start <= token.start
              ? state.completion.start
              : token.start;
          state.input = state.input.slice(0, start) + selected.word;
          __ezcSetInput(state, state.input);
          state.completion = null;
          __ezcHideMenu(state);
          return;
        }

        state.completion = null;
        __ezcHideMenu(state);
        submitLine();
        return;
      }

      if (data === "\u0003") {
        state.completion = null;
        __ezcHideMenu(state);
        state.input = "";
        state.term.write("^C\r\n" + state.prompt);
        return;
      }

      if (data === "\u000c") {
        state.completion = null;
        __ezcHideMenu(state);
        state.term.clear();
        state.term.write(state.prompt + state.input);
        return;
      }

      if (data === "\u001b") {
        state.completion = null;
        __ezcHideMenu(state);
        return;
      }

      if (data === "\t") {
        const token = __ezcTokenAtEnd(state.input);
        const prefix = token.prefix;

        if (prefix.length === 0) {
          __ezcFlashMenu(state, "type an operator or :command prefix");
          return;
        }

        const rebuildCompletion = () => {
          const matches = state.completions.filter((entry) =>
            entry.word.startsWith(prefix)
          );
          if (matches.length === 0) {
            state.completion = null;
            __ezcFlashMenu(state, `no completion for \`${prefix}\``);
            return false;
          }
          state.completion = {
            start: token.start,
            prefix,
            matches,
            index: 0,
          };
          return true;
        };

        if (!state.completion || state.completion.start !== token.start) {
          if (!rebuildCompletion()) {
            return;
          }
        } else {
          const current = state.completion.matches[state.completion.index];
          const selectedWord = current ? current.word : "";

          if (prefix === selectedWord || prefix === state.completion.prefix) {
            state.completion.index =
              (state.completion.index + 1) % state.completion.matches.length;
          } else {
            if (!rebuildCompletion()) {
              return;
            }
          }
        }

        const selected = state.completion.matches[state.completion.index];
        state.input = state.input.slice(0, token.start) + selected.word;
        __ezcSetInput(state, state.input);
        __ezcRenderMenu(state);
        return;
      }

      if (data === "\u001b[Z") {
        if (
          !state.completion ||
          !state.completion.matches ||
          state.completion.matches.length === 0
        ) {
          return;
        }
        state.completion.index =
          (state.completion.index - 1 + state.completion.matches.length) %
          state.completion.matches.length;
        const selected = state.completion.matches[state.completion.index];
        state.input = state.input.slice(0, state.completion.start) + selected.word;
        __ezcSetInput(state, state.input);
        __ezcRenderMenu(state);
        return;
      }

      if (data === "\u007f") {
        state.completion = null;
        __ezcHideMenu(state);
        if (state.input.length > 0) {
          state.input = state.input.slice(0, -1);
          state.term.write("\b \b");
        }
        return;
      }

      if (data === "\u001b[A") {
        state.completion = null;
        __ezcHideMenu(state);
        if (state.history.length === 0) {
          return;
        }
        if (state.historyIndex > 0) {
          state.historyIndex -= 1;
        }
        __ezcSetInput(state, state.history[state.historyIndex] || "");
        return;
      }

      if (data === "\u001b[B") {
        state.completion = null;
        __ezcHideMenu(state);
        if (state.history.length === 0) {
          return;
        }
        if (state.historyIndex < state.history.length - 1) {
          state.historyIndex += 1;
          __ezcSetInput(state, state.history[state.historyIndex] || "");
        } else {
          state.historyIndex = state.history.length;
          __ezcSetInput(state, "");
        }
        return;
      }

      state.completion = null;
      __ezcHideMenu(state);
      for (const ch of data) {
        if (__ezcPrintable(ch)) {
          state.input += ch;
          state.term.write(ch);
        }
      }
    });

    __ezcResize(state);
    window.addEventListener("resize", () => __ezcResize(state));

    term.writeln("EZC web REPL (persistent stack)");
    term.writeln("Commands: :help :clear :reset | Tab completion enabled");
    term.write(state.prompt);
  };

  bootstrap();
}

export function ezcTerminalSetCompletions(containerId, catalog) {
  __ezcTermPendingCompletions[containerId] = String(catalog ?? "");
  const state = __ezcTermRegistry[containerId];
  if (!state) {
    return;
  }
  state.completions = __ezcParseCompletions(catalog);
  state.completion = null;
  __ezcHideMenu(state);
}

export function ezcTerminalPrint(containerId, text) {
  const state = __ezcTermRegistry[containerId];
  if (!state) {
    return;
  }

  const payload = String(text ?? "");
  if (payload.length > 0) {
    for (const line of payload.split("\n")) {
      state.term.writeln(line);
    }
  }

  state.locked = false;
  state.completion = null;
  __ezcHideMenu(state);
  state.term.write(state.prompt);
}

export function ezcTerminalClear(containerId) {
  const state = __ezcTermRegistry[containerId];
  if (!state) {
    return;
  }

  state.locked = false;
  state.completion = null;
  __ezcHideMenu(state);
  state.input = "";
  state.term.clear();
  state.term.write(state.prompt);
}
"##)]
extern "C" {
    #[wasm_bindgen(js_name = ezcTerminalBoot)]
    fn js_terminal_boot(container_id: &str, bridge_id: &str);

    #[wasm_bindgen(js_name = ezcTerminalSetCompletions)]
    fn js_terminal_set_completions(container_id: &str, catalog: &str);

    #[wasm_bindgen(js_name = ezcTerminalPrint)]
    fn js_terminal_print(container_id: &str, text: &str);

    #[wasm_bindgen(js_name = ezcTerminalClear)]
    fn js_terminal_clear(container_id: &str);
}

#[cfg(target_arch = "wasm32")]
fn terminal_boot(container_id: &str, bridge_id: &str) {
    js_terminal_boot(container_id, bridge_id);
}

#[cfg(target_arch = "wasm32")]
fn terminal_set_completions(container_id: &str, catalog: &str) {
    js_terminal_set_completions(container_id, catalog);
}

#[cfg(target_arch = "wasm32")]
fn terminal_print(container_id: &str, text: &str) {
    js_terminal_print(container_id, text);
}

#[cfg(target_arch = "wasm32")]
fn terminal_clear(container_id: &str) {
    js_terminal_clear(container_id);
}

#[cfg(not(target_arch = "wasm32"))]
fn terminal_boot(_container_id: &str, _bridge_id: &str) {}

#[cfg(not(target_arch = "wasm32"))]
fn terminal_set_completions(_container_id: &str, _catalog: &str) {}

#[cfg(not(target_arch = "wasm32"))]
fn terminal_print(_container_id: &str, _text: &str) {}

#[cfg(not(target_arch = "wasm32"))]
fn terminal_clear(_container_id: &str) {}

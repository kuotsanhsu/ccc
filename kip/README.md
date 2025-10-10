## Desing guidelines

- If JS is disabled, the page will render no differently.
    - Minimal interactivity can still be achieved with CSS and browser built-in functionalities for FORM etc.
    - TEMPLATE and SLOT could still be used? But, webcomponents that rely on JS would not be usable. Thus, don'y use webcomponents that rely on JS for page rendering.
    - `contenteditable` can still be possible statically.
- If JS is enabled, rich inspection will be possible.

## HTML/JS references

- https://developer.mozilla.org/en-US/docs/Web/HTML/Reference/Elements
- https://webaim.org/techniques/skipnav/
- https://stackoverflow.com/questions/7290504/which-html5-tag-should-i-use-to-mark-up-an-author-s-name
    - http://microformats.org/wiki/hcard
- https://backsideofdawn.github.io/csscolorfinder/
- https://www.myfonts.com/pages/whatthefont
    - [ITC Galliard](https://en.wikipedia.org/wiki/Galliard_(typeface))
- https://developer.mozilla.org/en-US/docs/Web/API/TrustedHTML
- https://developer.mozilla.org/en-US/docs/Web/API/Document/adoptedStyleSheets
    - https://developer.mozilla.org/en-US/docs/Web/API/CSSStyleSheet/insertRule
- https://developer.mozilla.org/en-US/docs/Web/HTML/Guides/Quirks_mode_and_standards_mode
    > The only purpose of `<!doctype html>` is to activate no-quirks mode. 
- https://developer.mozilla.org/en-US/docs/Web/API/Pointer_Lock_API
- https://developer.mozilla.org/en-US/docs/Web/API/Web_Animations_API
- https://developer.mozilla.org/en-US/docs/Web/HTML/Reference/Global_attributes/contenteditable
- https://developer.mozilla.org/en-US/docs/Web/API/Element/contextmenu_event
    - https://developer.mozilla.org/en-US/docs/Web/API/Element/auxclick_event
- https://developer.mozilla.org/en-US/docs/Web/HTML/Reference/Elements/dialog
- https://developer.mozilla.org/en-US/docs/Web/CSS/CSS_positioned_layout/Stacking_context

## ms-vscode.live-server

```
Safari can't open the page "http://127.0.0.1:3000/index.html". The error is: "Navigation" failed because the request was for an HTTP URL with HTTPS-Only enabled" (WebKitErrorDomain:305)
```

## Web Components

- https://stackoverflow.com/questions/72090155/what-web-component-features-are-not-supported-by-safari-desktop-and-safari-ios
- https://github.com/mdn/web-components-examples
- https://stackoverflow.com/questions/64304353/main-differences-between-lit-element-web-components-react/64307820#64307820

## Fonts

- https://css-tricks.com/snippets/css/system-font-stack/
- [Canela](https://commercialtype.com/catalog/canela/canela) use by Apple Books.
- [How to Load and Use Custom CSS Fonts: A Complete Guide](https://www.digitalocean.com/community/tutorials/how-to-load-and-use-custom-fonts-with-css)

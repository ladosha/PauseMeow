# DeskCat Publishing and Promotion Strategy

Last researched: October 4, 2026

## Recommended approach

1. Rename before launch.
2. Use a small promotional site as the canonical home.
3. Publish on GitHub, Microsoft Store, and Flathub.
4. Soft-launch in a few carefully chosen Reddit communities.
5. Use Product Hunt and Hacker News as launch events, not foundations.
6. Pitch Telegram channels only after the download experience is polished and there is some evidence of interest.

## Positioning

Do not market it as merely another Pomodoro app. The memorable pitch is:

> A work timer that sends a cat to take over every monitor when it is time to rest.

A slightly fuller version:

> A free, local break timer for Windows and Linux. When your work interval ends, an animated cat dims every monitor, lies down, purrs, and stays until your break is over.

The cat entering, reacting, sleeping, and leaving is the product. The existing animation is already the strongest marketing asset.

## Rename before launch

"DeskCat" is crowded. There is already:

- a [Deskcat game on Steam](https://store.steampowered.com/app/3926040/Deskcat/);
- a [DeskCat Android app](https://play.google.com/store/apps/details?id=com.velvetvision.deskcat);
- an active [DeskCat desktop companion on GitHub](https://github.com/ppxinyue/DeskCat);
- several other similarly named desktop-cat projects.

That will hurt searchability, domain availability, Store approval and name reservation, and recommendations such as "download DeskCat."

Possible directions, not legally cleared names:

- Purrmoro
- BreakCat
- PurrBreak
- Cat Says Rest
- The Break Supervisor
- Mandatory Cat Break

"Purrmoro" is cute and immediately suggests a timer. "Cat Says Rest" is unusually searchable and explains the personality. Before committing, check Microsoft Store, GitHub, domains, social handles, and trademarks.

## Microsoft Store

Publish in the Microsoft Store, using an MSIX package if feasible.

For general Windows users, "Install from Microsoft Store" is more trustworthy than downloading an unfamiliar EXE from GitHub. Microsoft currently offers free individual developer registration. With MSIX, Microsoft hosts and signs the package and provides Store-managed updates. An EXE/MSI submission requires the developer to host a permanent versioned URL, sign the installer and included executables with a trusted certificate, and manage updates independently. See [Microsoft's official distribution comparison](https://learn.microsoft.com/en-us/windows/apps/package-and-deploy/choose-distribution-path).

Recommended distribution:

- Microsoft Store/MSIX as the main Windows button.
- GitHub Releases for technical users, portable builds, source, changelogs, and older versions.
- WinGet after release, so users can install it from the terminal. The [WinGet submission process](https://github.com/microsoft/winget-pkgs/blob/master/doc/README.md) is free and based on a manifest pull request.
- Flathub as the main Linux destination, with AppImage on GitHub as the fallback. The existing Flatpak manifest provides a head start; Flathub accepts new apps through a reviewed pull request process. See the [Flathub submission documentation](https://docs.flathub.org/docs/for-app-authors/submission).

Reserve the final Microsoft Store name immediately after renaming, even if the package comes slightly later.

## Promotional page

Build a small, charming, one-page site. GitHub Pages supports free hosting and custom domains for public repositories. See the [GitHub Pages documentation](https://docs.github.com/en/pages/configuring-a-custom-domain-for-your-github-pages-site/about-custom-domains).

Recommended structure:

1. Hero: looping 8-12 second video of the cat entering and settling down.
2. Headline: "Your break starts when the cat says so."
3. Buttons:
   - Get it from Microsoft
   - Download for Linux
   - View source on GitHub
4. Three benefits:
   - Impossible-to-miss breaks on every monitor
   - Strict or gentle mode
   - Local, lightweight, no account
5. Short interaction clip: click, annoyed reaction, waking up.
6. Privacy and trust section.
7. Donation button: "Buy the cat a treat."
8. FAQ covering autostart, multi-monitor support, uninstalling, Wayland limitations, and sound controls.

Use the page as the canonical URL everywhere. Its buttons should route visitors to Store, Flathub, and GitHub rather than presenting three unrelated home pages.

## Donation model

Combine:

- A Ko-fi or Buy Me a Coffee link on the website and in the app's About window.
- A GitHub sponsor button through `.github/FUNDING.yml`; GitHub supports Ko-fi, Buy Me a Coffee, GitHub Sponsors, and custom funding URLs. See the [GitHub funding-link documentation](https://docs.github.com/en/repositories/managing-your-repositorys-settings-and-features/customizing-your-repository/displaying-a-sponsor-button-in-your-repository).
- Optionally, an itch.io mirror configured as `$0 or Donate`. itch.io explicitly supports free downloads with a suggested contribution and hosts desktop application files. See the [itch.io pricing documentation](https://itch.io/docs/creators/pricing).

The donation prompt should remain playful and passive. Avoid modals, countdowns, missing functionality, or repeated nagging.

Before choosing a payment service, confirm that its payout method works for the developer's country and bank.

## Promotion channels

### Reddit: highest priority

Use native video or GIF posts, disclose that the app is self-made, and ask one genuine question. Do not post identical promotional copy into many communities.

Candidate communities:

- `r/pomodoro`
- `r/SideProject`
- `r/imadethis`
- `r/ProductivityApps`
- `r/linuxapps` for a Linux-specific launch
- `r/WindowsApps` for a Windows/Store-specific launch
- `r/software` only in its Self-Promotion Wednesday format

Avoid `r/productivity`: its current moderators explicitly prohibit app advertising and disguised feedback posts. See the [`r/productivity` announcement](https://www.reddit.com/r/productivity/comments/1simw4i/no_advertising_is_allowed_of_any_kind_including/). `r/software` currently confines creator posts to Wednesday and requires useful context beyond a dumped link. See an [`r/software` example](https://www.reddit.com/r/software/comments/1lptr84).

Possible title:

> I made a break timer where a cat takes over every monitor until you rest [Windows/Linux, free]

The post should tell the short origin story, show the clip, list platform and privacy information, and ask something specific, such as:

> Is strict mode motivating, or would you immediately disable it?

Check each subreddit's rules again on posting day because they change frequently.

### Product Hunt: worthwhile but secondary

Product Hunt is free and well suited to polished, delightful projects. It rewards strong visuals and novelty, which this app has. However, it is mostly a one-day burst, so launch there only after Store links, landing page, and onboarding are ready.

Product Hunt recommends posting your own product and using a direct product URL; current accounts must also be at least one week old. See the [official posting guide](https://help.producthunt.com/en/articles/479557-how-to-post-a-product) and [official launch guide](https://www.producthunt.com/launch).

Suggested tagline:

> A break timer that sends in the cat.

### Hacker News: optional technical launch

A Show HN could work if the source is public and the post discusses multi-monitor overlays, accurate timers across sleep and resume, Qt packaging, and Linux compositor limitations.

Suggested title:

> Show HN: A cat that takes over every monitor when it is time for a break

Hacker News requires something people can actually try, discourages launch pages and vote solicitation, and expects authors to participate in the discussion. See the [Show HN guidelines](https://news.ycombinator.com/showhn.html).

### Directories

Use only a few:

- Product Hunt for launch-day exposure.
- AlternativeTo for durable discovery; creators can [submit their own software](https://alternativeto.net/faq/).
- itch.io for the cute indie audience and pay-what-you-want downloads.
- Microsoft Store, WinGet, and Flathub for actual distribution.

Avoid mass-submitting to generic startup directories. Most will produce negligible qualified traffic and create stale listings that require maintenance.

## Telegram strategy

Telegram should be earned-media outreach rather than the first launch step.

Two particularly relevant targets are:

- [Software | Sofты | Программы](https://tgboard.com/ru/channels/channels-tech/software_softs), which covers free and open software for Windows, macOS, and Linux.
- [Rozetked](https://t.me/s/rozetked?before=21610), which accepts news through `@rozetkednewsbot` and regularly features useful desktop applications.

Start with smaller software and indie-development channels. If one covers the app and users respond well, pitch larger channels with that evidence.

Suggested Russian pitch:

> Сделал бесплатный таймер перерывов для Windows и Linux. Когда рабочее время заканчивается, на всех мониторах появляется кот, затемняет экран, укладывается и мурчит до конца перерыва. Работает локально, без аккаунта и телеметрии. Вот 12-секундное видео, страница проекта и исходники.

Attach rather than merely link:

- a 10-15 second captioned MP4;
- one clean screenshot;
- Store or download link;
- GitHub link;
- three factual bullets;
- contact details.

Send personal pitches to roughly 8-12 well-matched channels. Follow up once, after about a week. Do not buy broad Telegram advertising until there is evidence that visitors actually download the app.

## Release work before promotion

The repository currently has working Windows, AppImage, and Flatpak artifacts, but it is still version `0.1.0`, has no Git remote, and lacks a root license, privacy statement, and third-party notices. The Linux metadata currently labels the application proprietary.

Before sending traffic:

- Choose the public name.
- Decide whether it is genuinely open source or merely source-visible.
- Add an explicit code license and separate art/audio licensing.
- Document ownership and provenance of the sprites and sounds.
- Add privacy, security, contribution, and support information.
- Publish a tagged release with checksums.
- Test clean install, update, autostart, and uninstall paths.
- Test Windows with SmartScreen and Store installation.
- Test at least GNOME Wayland, KDE Wayland, and X11.
- Add an issue template that collects OS, desktop environment, monitor count, and logs.

Qt licensing also deserves attention before Store publication. Qt's LGPL path requires notices, license text, access to corresponding Qt source, relinking rights, and other obligations; Qt notes that some application-store terms can conflict with LGPL distribution. See [Qt's official LGPL obligations](https://www.qt.io/development/open-source-lgpl-obligations). This does not necessarily prevent Store publication, but it should be resolved deliberately before launch.

## Lightweight launch schedule

### Week 1

- Rename and reserve the Store name.
- Finalize licensing and asset rights.
- Create the GitHub repository, landing page, donation link, and press kit.
- Produce Windows and Linux beta releases.

### Week 2

- Recruit 15-30 testers through `r/SideProject`, `r/pomodoro`, friends, and Linux communities.
- Fix installation, multi-monitor, autostart, and Wayland problems.
- Collect a few useful quotes with permission.

### Week 3

- Publish v1.0 to Microsoft Store, GitHub, WinGet, and Flathub.
- Launch coordinated but separately written Reddit posts.
- Submit to AlternativeTo and itch.io.

### Week 4

- Launch on Product Hunt and possibly Show HN.
- Send individualized Telegram pitches.
- Post a technical or build story instead of another generic announcement.

## Guiding principle

The strategy is not to be everywhere. It is to make one highly shareable cat clip, place the app in trusted stores, and give communities a funny story worth passing along.

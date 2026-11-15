// The string table: X(id, English, French) for the screens, P(id, English, French) for the web
// pages (their templates say `%Id%`). Included by i18n.h and i18n.cpp only.
// French: vous or neutral imperatives, a no-break space (U+00A0) before ? ! : ; and the typographic
// apostrophe (’), which also keeps page strings safe inside the pages' JavaScript quotes.
// Every language of an id takes the same printf arguments (test/test_i18n).

// Shared
X(Close, "Close", "Fermer")
X(Cancel, "Cancel", "Annuler")
X(Restart, "Restart", "Redémarrer")
X(Done, "Done", "Terminé")
X(Show, "Show", "Afficher")
X(Later, "Later", "Plus tard")
X(Searching, "searching...", "recherche…")

// Settings screen
X(Settings, "Settings", "Réglages")
X(PageAddress, "%s at %s  -  addresses and feeds are typed there, with the code this screen shows.",
  "%s, à l’adresse %s  -  adresses et flux se saisissent sur cette page, avec le code affiché ici.")
X(LyricsNotLooked, "No lyrics are looked up.", "Aucune recherche de paroles.")
X(LyricsLookedOn, "Looked up on %s.", "Recherchées sur %s.")
X(LyricsLookedOnCompanion, "Looked up on a TextFromTrack companion found on the network.",
  "Recherchées sur un compagnon TextFromTrack trouvé sur le réseau.")
X(SourceDirectTitle, "Talk to the music server?", "Passer par le serveur de musique ?")
X(SourceDirectText,
  "The panel restarts, finds the server itself and asks you to enable it there once. Lyrics then come "
  "from wherever the Lyrics setting points.",
  "Le panneau redémarre, trouve lui-même le serveur et vous demande une fois de l’y autoriser. Les "
  "paroles viennent alors de la source choisie dans le réglage Paroles.")
X(SourceCompanionTitle, "Use the companion?", "Utiliser le compagnon ?")
X(SourceCompanionText,
  "The panel restarts and looks for the TextFromTrack companion on the network, which is the only "
  "source that reads the lyrics already in your own files.",
  "Le panneau redémarre et cherche le compagnon TextFromTrack sur le réseau : c’est la seule "
  "source qui lit les paroles déjà présentes dans vos propres fichiers.")
X(ForgetWifiTitle, "Forget Wi-Fi?", "Oublier le Wi-Fi ?")
X(ForgetWifiText, "The panel leaves this network and shows its setup QR codes again.",
  "Le panneau quitte ce réseau et affiche de nouveau ses QR codes d’installation.")
X(Forget, "Forget", "Oublier")
X(LanguageTitle, "Switch to English?", "Passer en français ?")
X(LanguageText, "The panel restarts in English.", "Le panneau redémarre en français.")
X(Language, "Language", "Langue")
X(LanguageChoices, "English\nFrançais", "English\nFrançais")
X(Brightness, "Brightness", "Luminosité")
X(Warmth, "Colour warmth", "Chaleur des couleurs")
X(WarmthSub, "Warmer counters the panel's bluish white.", "Plus chaud, il corrige le blanc bleuté de l’écran.")
X(ScreenOff, "Screen off after", "Écran éteint après")
X(ScreenOffSub, "Dims after 1 min without playback, then turns off. A touch wakes it.",
  "S’assombrit après 1 min sans lecture, puis s’éteint. Touchez-le pour le réveiller.")
X(SleepChoices, "Never\n5 min\n10 min\n30 min\n60 min", "Jamais\n5 min\n10 min\n30 min\n60 min")
X(LyricsAhead, "Lyrics ahead by", "Avance des paroles")
X(LyricsAheadSub, "Shows each line slightly before it is sung.", "Affiche chaque ligne un peu avant qu’elle soit chantée.")
X(Backdrop, "Blurred cover behind the lyrics", "Pochette floutée derrière les paroles")
X(SmoothScroll, "Smooth lyrics scroll", "Défilement fluide des paroles")
X(ClockIdle, "Clock when nothing plays", "Horloge quand rien n’est lu")
X(TimeZone, "Time zone", "Fuseau horaire")
X(TzNames,
  "US Eastern\nUS Central\nUS Mountain\nUS Arizona\nUS Pacific\nUK, Ireland, Portugal\nCentral Europe (Paris)\n"
  "Eastern Europe\nJapan\nAustralia East\nUTC",
  "États-Unis Est\nÉtats-Unis Centre\nÉtats-Unis Rocheuses\nÉtats-Unis Arizona\nÉtats-Unis Pacifique\n"
  "Londres, Dublin, Lisbonne\nEurope centrale (Paris)\nEurope de l’Est\nJapon\nAustralie Est\nUTC")
X(TzCustom, "Custom", "Personnalisé")
X(ColourTest, "Colour test", "Mire de couleurs")
X(ColourTestSub, "White, greys and colours, to set the warmth by eye.",
  "Blanc, gris et couleurs, pour régler la chaleur à l’œil.")
X(WarmthValue, "Warmth %d %%", "Chaleur %d %%")
X(ColourNames, "White\nRed\nGreen\nBlue\nYellow\nCyan\nMagenta\nSkin\nOlive",
  "Blanc\nRouge\nVert\nBleu\nJaune\nCyan\nMagenta\nPeau\nOlive")
X(MusicFrom, "Music from", "Source de la musique")
X(MusicFromSub,
  "The companion reads your own files and ranks what it finds. Direct talks to the music server "
  "itself, with nothing in between.",
  "Le compagnon lit vos propres fichiers et classe ce qu’il trouve. En direct, le panneau parle au "
  "serveur de musique lui-même, sans intermédiaire.")
X(SourceChoices, "TextFromTrack companion\nThe music server, directly",
  "Le compagnon\nLe serveur, en direct")
X(Lyrics, "Lyrics", "Paroles")
X(LyricsChoices, "A companion on the network\nNo lyrics", "Compagnon sur le réseau\nPas de paroles")
X(Zone, "Zone", "Zone")
X(ZoneSub, "Which Roon zone the panel follows.", "La zone Roon que suit le panneau.")
X(Choose, "Choose", "Choisir")
X(NotSet, "(not set)", "(non configuré)")
X(ChangeDots, "Change...", "Changer…")
X(ForgetDots, "Forget...", "Oublier…")
X(SettingsPage, "Settings page", "Page de réglages")
X(QrCode, "QR code", "QR code")
X(Firmware, "Firmware", "Logiciel")
X(FirmwareVersion, "Firmware  %s", "Logiciel  %s")
X(CheckNow, "Check now", "Vérifier")
X(About, "About", "À propos")
X(AboutText,
  "Panel %s.local  (%s)\nMusic server %s\nFree internal RAM %s\nSource, guide and releases: github.com/r45635/tft-panel",
  "Panneau %s.local  (%s)\nServeur de musique %s\nRAM interne libre %s\nSources, guide et versions : "
  "github.com/r45635/tft-panel")
X(FwIdle, "Checks for updates by itself every day.", "Cherche lui-même les mises à jour chaque jour.")
X(FwChecking, "Checking for updates...", "Recherche de mises à jour…")
X(FwUpToDate, "Up to date", "À jour")
X(FwUpToDateLatest, "Up to date (latest %s)", "À jour (dernière version : %s)")
X(FwAvailable, "%s is available.", "La version %s est disponible.")
X(FwDownloading, "Downloading %s... %d %%", "Téléchargement de %s… %d %%")
X(FwInstalled, "Installed, restarting...", "Installée, redémarrage…")
X(FwFailed, "Last attempt failed: %s", "Échec de la dernière tentative : %s")
X(FwInstall, "Install %s", "Installer %s")

// Main screen, setup, popups, clock
X(SetupTitle, "Set up TFT Panel", "Configurer TFT Panel")
X(SetupScanWifi, "1. Scan to join the panel's Wi-Fi", "1. Scannez pour rejoindre le Wi-Fi du panneau")
X(SetupScanPage, "2. Scan to open the setup page", "2. Scannez pour ouvrir la page de configuration")
X(SetupHere, "Set up on this screen", "Configurer sur cet écran")
X(SetupByHand, "Or join by hand:\n\nNetwork\n%s\n\nPassword\n%s\n\nthen open\n%s",
  "Ou à la main :\n\nRéseau\n%s\n\nMot de passe\n%s\n\npuis ouvrez\n%s")
X(ZoneFollow, "Follow the playing zone (auto)", "Suivre la zone en lecture (auto)")
X(ZoneStates, "playing\npaused\nstopped\nloading", "en lecture\nen pause\narrêtée\nchargement")
X(Mute, "Mute", "Couper le son")
X(Unmute, "Unmute", "Rétablir le son")
X(Volume, "Volume", "Volume")
X(VolumeButtonsOnly, "Volume (this zone only has - / +)", "Volume (cette zone n’a que - / +)")
X(Muted, "muted", "coupé")
X(InSync, "In sync", "Synchronisées")
X(OffsetLater, "%.2f s later", "%.2f s plus tard")
X(OffsetEarlier, "%.2f s earlier", "%.2f s plus tôt")
X(OffsetTitle, "Lyrics timing for this track", "Décalage des paroles pour ce titre")
X(Earlier, "Earlier", "Plus tôt")
X(Reset, "Reset", "Remettre à zéro")
X(CodeText, "This panel's web page\n\nType this code there to allow the change (valid 5 minutes):",
  "Page web de ce panneau\n\nSaisissez-y ce code pour valider le changement (5 minutes) :")
X(InstallNow, "Install now", "Installer maintenant")
X(OtaBadge, "Update %s", "Mise à jour %s")
X(OtaConfirm,
  "Install TFT Panel %s?\n\nIt downloads in about a minute, then the panel restarts. If the new version "
  "does not start properly, the panel goes back to %s by itself.",
  "Installer TFT Panel %s ?\n\nLe téléchargement prend environ une minute, puis le panneau redémarre. Si "
  "la nouvelle version ne démarre pas correctement, le panneau revient de lui-même à la version %s.")
X(OtaUpdating, "Updating to %s\nDo not unplug the panel", "Mise à jour vers %s\nNe débranchez pas le panneau")
X(OtaFailed, "Update failed: %s\n\nStill running %s.", "Échec de la mise à jour : %s\n\nLe panneau reste en version %s.")
X(LyricsLoading, "Looking for lyrics...", "Recherche des paroles…")
X(Instrumental, "Instrumental", "Instrumental")
X(LyricsNotFound, "No lyrics found for this track", "Aucune parole trouvée pour ce titre")
X(LyricsUnavailable, "Lyrics unavailable: %s", "Paroles indisponibles : %s")
X(WeatherToday, "%d to %d °C today", "%d à %d °C aujourd’hui")
X(Days, "Sunday\nMonday\nTuesday\nWednesday\nThursday\nFriday\nSaturday",
  "dimanche\nlundi\nmardi\nmercredi\njeudi\nvendredi\nsamedi")
X(Months, "January\nFebruary\nMarch\nApril\nMay\nJune\nJuly\nAugust\nSeptember\nOctober\nNovember\nDecember",
  "janvier\nfévrier\nmars\navril\nmai\njuin\njuillet\naoût\nseptembre\noctobre\nnovembre\ndécembre")
X(PausedPrefix, "Paused  -  ", "En pause  -  ")

// Library
X(Loading, "Loading...", "Chargement…")
X(ShowMore, "Show more (%d left)", "Afficher plus (%d restants)")
X(NothingHere, "Nothing here", "Rien ici")
X(Search, "Search", "Rechercher")
X(Browse, "Browse", "Parcourir")
X(SearchHint, "Artist, album, track...", "Artiste, album, titre…")
X(PlaysOn, "Plays on: %s", "Lecture sur : %s")
X(NoZone, "No zone: pick one in the controls bar", "Aucune zone : choisissez-en une dans la barre de commandes")

// Wi-Fi screen
X(SignalWords, "Excellent\nGood\nFair\nWeak", "Excellent\nBon\nMoyen\nFaible")
X(WifiSaved, "Saved. The panel restarts and joins\n%s", "Enregistré. Le panneau redémarre et rejoint\n%s")
X(WifiPassLength, "A Wi-Fi password is 8 to 63 characters.", "Un mot de passe Wi-Fi compte 8 à 63 caractères.")
X(PasswordFor, "Password for %s", "Mot de passe de %s")
X(WifiOpen, "open", "ouvert")
X(WifiSecured, "secured", "sécurisé")
X(WifiLooking, "Looking for networks...", "Recherche des réseaux…")
X(WifiNoneHeard, "No network heard. Move the panel closer to the router, then refresh.",
  "Aucun réseau capté. Rapprochez le panneau du routeur, puis actualisez.")
X(WifiChoose, "Choose your Wi-Fi", "Choisissez votre Wi-Fi")
X(WifiNote, "2.4 GHz networks. Hidden network: use the setup page on a phone.",
  "Réseaux 2,4 GHz. Réseau masqué : page de configuration sur un téléphone.")
X(Password, "Password", "Mot de passe")
X(ShowPassword, "Show", "Voir")
X(HidePassword, "Hide", "Cacher")
X(Back, "Back", "Retour")
X(Connect, "Connect", "Se connecter")

// Web pages: messages built in code
X(WebWrongCode, "Wrong or expired code: open this page again to get a new one on the panel.",
  "Code erroné ou expiré : rouvrez cette page pour en obtenir un nouveau sur le panneau.")
X(WebNoMemory, "Not enough memory to receive the firmware.", "Pas assez de mémoire pour recevoir le logiciel.")
X(WebCannotStart, "Cannot start the update: %s", "Impossible de lancer la mise à jour : %s")
X(WebTooLarge, "File too large for a firmware slot.", "Fichier trop gros pour un emplacement de logiciel.")
X(WebWriteFailed, "Write failed: %s", "Échec de l’écriture : %s")
X(WebChecksum, "Checksum does not match: nothing installed. The panel received sha256 %s, you asked for %s.",
  "La somme de contrôle ne correspond pas : rien n’a été installé. Le panneau a reçu le sha256 %s, vous "
  "attendiez %s.")
X(WebChecksumScreen, "That file is not the one you asked for: nothing installed.",
  "Ce fichier n’est pas celui attendu : rien n’a été installé.")
X(WebNotImage, "Not a valid firmware image: %s", "Ce n’est pas une image de logiciel valide : %s")
X(WebInterrupted, "Upload interrupted, nothing installed.", "Envoi interrompu, rien n’a été installé.")
X(WebInstalled,
  "Installed (sha256 %s). The panel restarts now; it goes back to the previous version by itself if this one "
  "does not start properly.",
  "Installé (sha256 %s). Le panneau redémarre ; il revient de lui-même à la version précédente si "
  "celle-ci ne démarre pas correctement.")
X(WebNoFirmware, "No firmware received.", "Aucun logiciel reçu.")
X(WebRemembered, "This browser is remembered, so no code is needed.",
  "Ce navigateur est mémorisé : aucun code n’est nécessaire.")
X(WebCodeLabel, "Code shown on this panel's screen", "Code affiché sur l’écran de ce panneau")
X(WebCodeDigits, "6 digits", "6 chiffres")
X(WebForgetBrowser, "Forget this browser", "Oublier ce navigateur")
X(WebShowing, "Showing %s", "Affiché : %s")
X(WebCodeRefused,
  "That code is wrong or has expired. Nothing was saved. The panel's screen shows a new code; what you "
  "typed is still here.",
  "Ce code est erroné ou a expiré. Rien n’a été enregistré. L’écran du panneau affiche un nouveau code ; "
  "ce que vous avez saisi est toujours là.")
X(WebBadCompanion, "Nothing was saved. The companion address starts with http://",
  "Rien n’a été enregistré. L’adresse du compagnon commence par http://")
X(WebBadLyrics, "Nothing was saved. The lyrics server starts with http:// or https://, or is the word none.",
  "Rien n’a été enregistré. Le serveur de paroles commence par http:// ou https://, ou c’est le mot none.")
X(WebBadNews, "Nothing was saved. The news feed starts with http:// or https://",
  "Rien n’a été enregistré. Le flux d’actualités commence par http:// ou https://")
X(WebChangedNames,
  "music source\ncompanion address\ncompanion token\nlyrics\nlyrics token\nweather town\nnews feed\nlanguage",
  "source de la musique\nadresse du compagnon\njeton du compagnon\nparoles\njeton des paroles\n"
  "ville de la météo\nflux d’actualités\nlangue")
X(WebSaved, "Saved", "Enregistré")
X(WebRestartSource, "The panel restarts to use its new source.", "Le panneau redémarre pour utiliser sa nouvelle source.")
X(WebSavedList, "Saved: %s.", "Enregistré : %s.")
X(WebNothingToChange, "Nothing to change; all the fields already said that.",
  "Rien à changer : tous les champs disaient déjà cela.")
X(WebNotForgotten, "Nothing was forgotten: this browser was not remembered.",
  "Rien n’a été oublié : ce navigateur n’était pas mémorisé.")
X(WebForgotten, "Forgotten. The next change asks for the code on the panel's screen.",
  "C’est oublié. La prochaine modification demandera le code affiché sur l’écran du panneau.")

// Web pages: template text (`%Id%` in SETTINGS_PAGE and PAGE). HTML is allowed. PgUpPick to PgUpNotSha
// sit inside JavaScript quotes: no ASCII ' there.
P(PgSource, "Where the music information comes from", "D’où viennent les informations sur la musique")
P(PgSourceCompanion, "The TextFromTrack companion", "Le compagnon TextFromTrack")
P(PgSourceDirect, "The music server, directly", "Le serveur de musique, en direct")
P(PgCompanion, "Companion address", "Adresse du compagnon")
P(PgCompanionHint, "(empty = found on the network)", "(vide = trouvé sur le réseau)")
P(PgToken, "Companion access token", "Jeton d’accès du compagnon")
P(PgTokenHint, "(only if it has one; empty leaves it unchanged)", "(seulement s’il en a un ; vide = inchangé)")
P(PgLyrics, "Lyrics", "Paroles")
P(PgLyricsHint,
  "(empty = a companion on the network, <code>none</code> = no lyrics,\n or the address of an LRCLIB-compatible server)",
  "(vide = un compagnon sur le réseau, <code>none</code> = pas de paroles,\n ou l’adresse d’un serveur compatible LRCLIB)")
P(PgLyricsToken, "Lyrics server token", "Jeton du serveur de paroles")
P(PgLyricsTokenHint, "(only if it asks for one)", "(seulement s’il en demande un)")
P(PgWeather, "Weather town", "Ville de la météo")
P(PgWeatherHint,
  "(add a region or country if the name is shared:\n <code>Montreal, Canada</code>, <code>Paris, Texas</code>, "
  "<code>Lyon, France</code>)",
  "(ajoutez une région ou un pays si le nom est partagé :\n <code>Montreal, Canada</code>, <code>Paris, Texas</code>, "
  "<code>Lyon, France</code>)")
P(PgNews, "News feed", "Flux d’actualités")
P(PgNewsHint, "(RSS or Atom, slid under the clock; empty = none)", "(RSS ou Atom, défile sous l’horloge ; vide = aucun)")
P(PgSave, "Save", "Enregistrer")
P(PgFooterUpdate, "Firmware update:", "Mise à jour du logiciel :")
P(PgFooterSource, "open source (MIT):", "code source libre (MIT) :")
P(PgUpTitle, "Update TFT Panel", "Mettre à jour TFT Panel")
P(PgUpIntro,
  "Running <b>%VERSION%</b>. Choose a firmware file (<code>tft-panel-vX.Y.Z.bin</code>)\nand type the code shown on "
  "the panel's screen.",
  "Version actuelle : <b>%VERSION%</b>. Choisissez un fichier de logiciel (<code>tft-panel-vX.Y.Z.bin</code>)\n"
  "et saisissez le code affiché sur l’écran du panneau.")
P(PgUpCode, "6-digit code", "Code à 6 chiffres")
P(PgUpSha, "sha256 from the release (optional)", "sha256 de la version (facultatif)")
P(PgUpShaHint,
  "Paste the line from <code>%ASSET%-vX.Y.Z.sha256</code> and the panel refuses the file if it does not match.\n"
  "The panel checks the image itself either way, and goes back to this version if the new one does not start.",
  "Collez la ligne de <code>%ASSET%-vX.Y.Z.sha256</code> : le panneau refuse le fichier s’il ne correspond pas.\n"
  "Dans tous les cas, le panneau vérifie l’image lui-même et revient à cette version si la nouvelle ne démarre pas.")
P(PgUpGo, "Upload and install", "Envoyer et installer")
P(PgUpReleases, "The firmware files and their sha256 are on the releases page,",
  "Les fichiers du logiciel et leur sha256 sont sur la page des versions,")
P(PgUpAsset,
  "This panel takes <code>%ASSET%-vX.Y.Z.bin</code> &mdash; the name matters: two different screens\ncan share the same chip.",
  "Ce panneau prend <code>%ASSET%-vX.Y.Z.bin</code> &mdash; le nom compte : deux écrans différents\npeuvent avoir la même puce.")
P(PgUpPick, "Pick a file and type the code.", "Choisissez un fichier et saisissez le code.")
P(PgUpUploading, "Uploading", "Envoi")
P(PgUpLost, "Upload failed (connection lost).", "Échec de l’envoi (connexion perdue).")
P(PgUpNotSha, "That does not look like a sha256: 64 hex characters.",
  "Ce n’est pas un sha256 : il faut 64 caractères hexadécimaux.")

// Setup portal (the panel's own Wi-Fi, before it has a network)
P(PgPortalTitle, "TFT Panel setup", "Configuration de TFT Panel")
X(PortalLooking, "Looking for networks&hellip; <a href=/>refresh</a>", "Recherche des réseaux&hellip; <a href=/>actualiser</a>")
X(PortalTap, "Tap your network <small>(2.4 GHz only)</small>", "Touchez votre réseau <small>(2,4 GHz uniquement)</small>")
X(PortalNotListed, "Not in the list? Type its name below, or <a href=/>refresh</a>.",
  "Il n’est pas dans la liste ? Saisissez son nom ci-dessous, ou <a href=/>actualisez</a>.")
X(PortalWifiNetwork, "Wi-Fi network", "Réseau Wi-Fi")
X(PortalWifiPassword, "Wi-Fi password", "Mot de passe Wi-Fi")
X(PortalUnchanged, "unchanged if left empty", "inchangé si laissé vide")
X(PortalSetUnchanged, "set — unchanged if left empty", "défini — inchangé si laissé vide")
X(PortalCompanionHint, "(optional, found automatically on the LAN)", "(facultatif, trouvé automatiquement sur le réseau local)")
X(PortalTokenHint, "(only if the companion has ACCESS_TOKEN)", "(seulement si le compagnon a un ACCESS_TOKEN)")
X(PortalLyricsServer, "Lyrics server", "Serveur de paroles")
X(PortalLyricsHint, "(direct only; empty = a companion on the network, <code>none</code> = no lyrics)",
  "(direct uniquement ; vide = un compagnon sur le réseau, <code>none</code> = pas de paroles)")
X(PortalSave, "Save and restart", "Enregistrer et redémarrer")
X(PortalBadSsid, "The network name must be 1 to 32 characters.", "Le nom du réseau compte 1 à 32 caractères.")
X(PortalBadPass, "A WPA password is 8 to 63 characters (leave it empty for an open network).",
  "Un mot de passe WPA compte 8 à 63 caractères (laissez-le vide pour un réseau ouvert).")
X(PortalBadCompanion, "The companion address starts with http:// (for example http://192.168.1.20:3888).",
  "L’adresse du compagnon commence par http:// (par exemple http://192.168.1.20:3888).")
X(PortalBadLyrics, "The lyrics server starts with http:// or https://, or is the word none.",
  "Le serveur de paroles commence par http:// ou https://, ou c’est le mot none.")
X(PortalSaved, "The panel restarts and joins <b>%s</b>. Reconnect this phone to your usual Wi-Fi.",
  "Le panneau redémarre et rejoint <b>%s</b>. Reconnectez ce téléphone à votre Wi-Fi habituel.")

// Weather, WMO 4677 as Open-Meteo uses it
X(WxClear, "Clear", "Dégagé")
X(WxMainlyClear, "Mainly clear", "Plutôt dégagé")
X(WxPartlyCloudy, "Partly cloudy", "Partiellement nuageux")
X(WxOvercast, "Overcast", "Couvert")
X(WxFog, "Fog", "Brouillard")
X(WxDrizzle, "Drizzle", "Bruine")
X(WxFreezingDrizzle, "Freezing drizzle", "Bruine verglaçante")
X(WxLightRain, "Light rain", "Pluie faible")
X(WxRain, "Rain", "Pluie")
X(WxHeavyRain, "Heavy rain", "Forte pluie")
X(WxFreezingRain, "Freezing rain", "Pluie verglaçante")
X(WxLightSnow, "Light snow", "Neige faible")
X(WxSnow, "Snow", "Neige")
X(WxHeavySnow, "Heavy snow", "Forte neige")
X(WxSnowGrains, "Snow grains", "Neige en grains")
X(WxShowers, "Showers", "Averses")
X(WxHeavyShowers, "Heavy showers", "Fortes averses")
X(WxSnowShowers, "Snow showers", "Averses de neige")
X(WxThunderstorm, "Thunderstorm", "Orage")
X(WxThunderstormWithHail, "Thunderstorm with hail", "Orage avec grêle")

// Status line and errors from the music source and the network
X(StCompanionToken, "Companion needs its access token (set it in the setup portal)",
  "Le compagnon demande son jeton d’accès (à saisir sur la page de configuration)")
X(StCompanionNoCore, "Companion: Roon Core not connected", "Compagnon : Roon Core non connecté")
X(StCompanionFailed, "Companion %s failed: %s", "Compagnon, échec de %s : %s")
X(StLookingCompanion, "Looking for the TextFromTrack companion on the network...",
  "Recherche du compagnon TextFromTrack sur le réseau…")
X(StNoCompanion, "No companion found on the network yet (is it running?)",
  "Aucun compagnon trouvé sur le réseau pour l’instant (est-il lancé ?)")
X(StCompanionNoZone, "The companion sees no Roon zone", "Le compagnon ne voit aucune zone Roon")
X(CmdControl, "Control %s", "Commande %s")
X(CmdFailed, "%s failed: %s", "%s : échec (%s)")
X(NewsNoAnswer, "That feed did not answer (%s). Trying again every %d minutes.",
  "Ce flux n’a pas répondu (%s). Nouvel essai toutes les %d minutes.")
X(NewsNoHeadline,
  "That address answered, but it holds nothing that looks like a headline. An RSS or Atom feed is wanted, not a web page.",
  "Cette adresse a répondu, mais rien n’y ressemble à un titre d’actualité. Il faut un flux RSS ou Atom, "
  "pas une page web.")
X(WeatherNoPlace, "No place called \"%s\". Try the town on its own, as in Montreal.",
  "Aucun lieu nommé « %s ». Essayez la ville seule, par exemple Montréal.")
X(StWifiChip, "Updating the Wi-Fi chip to %s - about a minute, do not unplug the panel",
  "Mise à jour de la puce Wi-Fi vers %s - environ une minute, ne débranchez pas le panneau")
X(StWifiChipPct, "Updating the Wi-Fi chip to %s: %d %% - do not unplug the panel",
  "Mise à jour de la puce Wi-Fi vers %s : %d %% - ne débranchez pas le panneau")

// Signed updates (web page)
X(WebSigLabel, "Signature", "Signature")
X(WebSigHint, "This panel installs only signed firmware: pick <code>%s-vX.Y.Z.sig</code> from the same release.",
  "Ce panneau n’installe que des logiciels signés : choisissez <code>%s-vX.Y.Z.sig</code> dans la même version.")
X(WebSigMismatch,
  "Signature does not match: nothing installed. This panel installs only firmware signed by its maintainer; pick "
  "the .sig published with this file.",
  "La signature ne correspond pas : rien n’a été installé. Ce panneau n’installe que des logiciels signés par "
  "son mainteneur ; choisissez le .sig publié avec ce fichier.")
X(WebSigMissing,
  "No signature: nothing installed. This panel installs only signed firmware: pick the .sig published beside the .bin.",
  "Pas de signature : rien n’a été installé. Ce panneau n’installe que des logiciels signés : choisissez le "
  ".sig publié à côté du .bin.")
X(WebSigMismatchScreen, "That file is not signed by the maintainer: nothing installed.",
  "Ce fichier n’est pas signé par le mainteneur : rien n’a été installé.")
X(WebSigMissingScreen, "That file came without its signature: nothing installed.",
  "Ce fichier est arrivé sans sa signature : rien n’a été installé.")
P(PgUpNotSig, "That is not a signature file: pick the .sig of this firmware.",
  "Ce n’est pas un fichier de signature : choisissez le .sig de ce logiciel.")

# Pokélike

Pokémon-inspired [roguelike](http://en.wikipedia.org/wiki/Roguelike) game. *In development.*

_As of now, this project has been tested on Mac OS X 10.9 and Ubuntu 12.04 x64._

_This project is not for profit. Please don't sue me for using copyrighted assets._

## Development Dependencies

- All platforms: clang, ncurses
- Linux: JACK Audio Connection Kit

## TODO

- [x] it's time to split up pokelike.c!
- [ ] sometimes battle doesn't render, obvious bug is alluding me at the moment (sleep time)
- [ ] deal with the "copyright infringement" (ie. move assets to submodule?) -> procedural music generation
- [ ] back-off when triggering a battle (so don't re-enter a battle immediately)
- [ ] flicker from render loop pretty bad on linux during battle state

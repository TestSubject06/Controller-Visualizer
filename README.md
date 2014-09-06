Controller-Visualizer
=====================

A contriller input visualizer for use with streaming

BUILD INFORMATION
=================
In order to build this, you'll need to get SFML (http://www.sfml-dev.org/) and link it into the project. There are tutorials on how to do this on their website. You'll also need to get a copy of the font Arial, and put it into the working directory of the code. The SFML Libraries use Boost(http://www.boost.org/), libsndfile(https://github.com/erikd/libsndfile) and OpenAL(http://kcat.strangesoft.net/openal.html) so you'll also need those headers linked in as well. Most of those come bundled with SFML, and Arial should be on your computer already, it just needs to be in the working directory, or use your own font, whatever.

This project was put together as a personal project to get rid of the need for a "hand-cam" when streaming. Instead of having an extra camera pointed at my hand, I wanted a window I could have on my other monitor and stream that as a separate source onto my stream. That window has an image of my controller that lights up and moves to match my physical controller. There were some people that wanted it, so I've been working to get it to a state where it can support multiple different controllers and layouts. It's close to being there.

https://www.youtube.com/watch?v=z9gTSNTeGsY is the old version of the vis in action.

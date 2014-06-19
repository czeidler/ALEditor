The Auckland Layout Editor (ALE)
====

The Auckland Layout Editor (ALE) is a GUI builder to create or edit constraint-based layouts. The constraint-based layout model is very powerful and can describe layouts that can't be described with other layout models, like for example, the grid-bag layout model. Layouts created with ALE are automatically resizable and non-overlapping. This means while editing a layout you can't create a layout that has two overlapping views. Furthermore, layouts created with ALE have no conflicting constraints.

ALE provides a small but powerful set of edit operations. These edit operations keep views automatically aligned to each other. This makes creating and editing layouts very easy.

For more information read our paper published at the [UIST'13](https://www.cs.auckland.ac.nz/~lutteroth/publications/ZeidlerEtAl2013-AucklandLayoutEditor.pdf) conference
or watch the [demo video](http://www.youtube.com/watch?v=ZPv58AWWGRQ").

Try ALE 
----
ALE is available in the Haiku application depot. Just run a [nightly Haiku image](http://www.haiku-files.org/unsupported-builds/x86-gcc4/), start HaikuDepot from the menu and install ALE.

If you want to build your own applications using a layout created with ALE take a look at the example code. To do so copy the example directory `/boot/system/data/ale/example` somewhere into your home directory and build it with:
```sh
cmake .
make
```

Build ALE
----

To build ALE just download the source code and build it with cmake:
```sh
cmake .
make
```
<html>
<body>
  <div id="readme" class="Box-body readme blob js-code-block-container p-5 p-xl-6 gist-border-0">
    <article class="markdown-body entry-content container-lg" itemprop="text"><h1><a id="user-content-freertos-emulator" class="anchor" aria-hidden="true" href="#freertos-emulator"><svg class="octicon octicon-link" viewBox="0 0 16 16" version="1.1" width="16" height="16" aria-hidden="true"><path fill-rule="evenodd" d="M7.775 3.275a.75.75 0 001.06 1.06l1.25-1.25a2 2 0 112.83 2.83l-2.5 2.5a2 2 0 01-2.83 0 .75.75 0 00-1.06 1.06 3.5 3.5 0 004.95 0l2.5-2.5a3.5 3.5 0 00-4.95-4.95l-1.25 1.25zm-4.69 9.64a2 2 0 010-2.83l2.5-2.5a2 2 0 012.83 0 .75.75 0 001.06-1.06 3.5 3.5 0 00-4.95 0l-2.5 2.5a3.5 3.5 0 004.95 4.95l1.25-1.25a.75.75 0 00-1.06-1.06l-1.25 1.25a2 2 0 01-2.83 0z"></path></svg></a>FreeRTOS Emulator</h1>
<p>
  <a href="https://travis-ci.com/alxhoff/FreeRTOS-Emulator" rel="nofollow">
  <img src="https://camo.githubusercontent.com/042cb7bcd463f4709df1aa9448cda08424f939d5/68747470733a2f2f7472617669732d63692e636f6d2f616c78686f66662f4672656552544f532d456d756c61746f722e7376673f6272616e63683d6d6173746572" data-canonical-src="https://travis-ci.com/alxhoff/FreeRTOS-Emulator.svg?branch=master" style="max-width:100%;">
  </a>
  <a href="https://github.com/alxhoff/FreeRTOS-Emulator/blob/master/LICENSE">
    <img src="https://camo.githubusercontent.com/dcb3a3de32cb31ae6a7edf80d88747f989878809/68747470733a2f2f696d672e736869656c64732e696f2f62616467652f6c6963656e73652d47504c76332d626c75652e737667" data-canonical-src="https://img.shields.io/badge/license-GPLv3-blue.svg" style="max-width:100%;">
  </a>
  <a href="https://scan.coverity.com/projects/alxhoff-freertos-emulator" rel="nofollow">
  <img alt="Coverity Scan Build Status" src="https://camo.githubusercontent.com/498dc92bf48514fb7db07f8b06f8b2345f72cc30/68747470733a2f2f7363616e2e636f7665726974792e636f6d2f70726f6a656374732f32303735372f62616467652e737667" data-canonical-src="https://scan.coverity.com/projects/20757/badge.svg" style="max-width:100%;">
  </a>
  <a href="https://codecov.io/gh/alxhoff/FreeRTOS-Emulator" rel="nofollow">
  <img src="https://camo.githubusercontent.com/7467513f9a0acbc35af0b1bca37049f2f5de36b4/68747470733a2f2f636f6465636f762e696f2f67682f616c78686f66662f4672656552544f532d456d756c61746f722f6272616e63682f6d61737465722f67726170682f62616467652e737667" data-canonical-src="https://codecov.io/gh/alxhoff/FreeRTOS-Emulator/branch/master/graph/badge.svg" style="max-width:100%;">
</a>
</p>
<p>An implementation of POSIX based FreeRTOS with the combination of SDL2 graphics. Aimed at providing an x86 emulation solution for teaching FreeRTOS to students without the need of embedded hardware. Used at the Technical University of Munich in the teaching of the "Embedded Systems Programming Lab". Please excuse any references to "students" or "course" if you're not one of my students.</p>
<p>Based on the FreeRTOS (V5.X) simulator developed by William Davy. Updated to use FreeRTOS V9.0.0.</p>
<p>Checkout the <a href="/alxhoff/FreeRTOS-Emulator/wiki">Wiki page</a> for a detailed Documentation!</p>
<p>Doxygen documentation can also be found on the GitHub Pages page.</p>
<h2><a id="user-content-dependencies" class="anchor" aria-hidden="true" href="#dependencies"><svg class="octicon octicon-link" viewBox="0 0 16 16" version="1.1" width="16" height="16" aria-hidden="true"><path fill-rule="evenodd" d="M7.775 3.275a.75.75 0 001.06 1.06l1.25-1.25a2 2 0 112.83 2.83l-2.5 2.5a2 2 0 01-2.83 0 .75.75 0 00-1.06 1.06 3.5 3.5 0 004.95 0l2.5-2.5a3.5 3.5 0 00-4.95-4.95l-1.25 1.25zm-4.69 9.64a2 2 0 010-2.83l2.5-2.5a2 2 0 012.83 0 .75.75 0 001.06-1.06 3.5 3.5 0 00-4.95 0l-2.5 2.5a3.5 3.5 0 004.95 4.95l1.25-1.25a.75.75 0 00-1.06-1.06l-1.25 1.25a2 2 0 01-2.83 0z"></path></svg></a>Dependencies</h2>
<p>The simulator uses the SDL2 graphics libraries.</p>
<h3><a id="user-content-debianubuntu" class="anchor" aria-hidden="true" href="#debianubuntu"><svg class="octicon octicon-link" viewBox="0 0 16 16" version="1.1" width="16" height="16" aria-hidden="true"><path fill-rule="evenodd" d="M7.775 3.275a.75.75 0 001.06 1.06l1.25-1.25a2 2 0 112.83 2.83l-2.5 2.5a2 2 0 01-2.83 0 .75.75 0 00-1.06 1.06 3.5 3.5 0 004.95 0l2.5-2.5a3.5 3.5 0 00-4.95-4.95l-1.25 1.25zm-4.69 9.64a2 2 0 010-2.83l2.5-2.5a2 2 0 012.83 0 .75.75 0 001.06-1.06 3.5 3.5 0 00-4.95 0l-2.5 2.5a3.5 3.5 0 004.95 4.95l1.25-1.25a.75.75 0 00-1.06-1.06l-1.25 1.25a2 2 0 01-2.83 0z"></path></svg></a>Debian/Ubuntu</h3>
<p>Assuming that you have some basic utilities like <code>make</code>, <code>cmake</code> and <code>git</code> already installed, execute:</p>
<div class="highlight highlight-source-shell"><pre>sudo apt-get install build-essential libsdl2-ttf-dev libsdl2-mixer-dev libsdl2-image-dev libsdl2-gfx-dev libsdl2-dev</pre></div>
<p>Additional requirements for development:</p>
<div class="highlight highlight-source-shell"><pre><span class="pl-c"><span class="pl-c">#</span> Depending on your OS version you might have to add the llvm-toolchain-4.0 APT source before</span>
sudo apt-get install clang-4.0 clang-tidy-4.0</pre></div>
<h3><a id="user-content-arch" class="anchor" aria-hidden="true" href="#arch"><svg class="octicon octicon-link" viewBox="0 0 16 16" version="1.1" width="16" height="16" aria-hidden="true"><path fill-rule="evenodd" d="M7.775 3.275a.75.75 0 001.06 1.06l1.25-1.25a2 2 0 112.83 2.83l-2.5 2.5a2 2 0 01-2.83 0 .75.75 0 00-1.06 1.06 3.5 3.5 0 004.95 0l2.5-2.5a3.5 3.5 0 00-4.95-4.95l-1.25 1.25zm-4.69 9.64a2 2 0 010-2.83l2.5-2.5a2 2 0 012.83 0 .75.75 0 001.06-1.06 3.5 3.5 0 00-4.95 0l-2.5 2.5a3.5 3.5 0 004.95 4.95l1.25-1.25a.75.75 0 00-1.06-1.06l-1.25 1.25a2 2 0 01-2.83 0z"></path></svg></a>Arch</h3>
<div class="highlight highlight-source-shell"><pre>sudo pacman -S sdl2 sdl2_gfx sdl2_image sdl2_mixer sdl2_ttf</pre></div>
<p>Additional requirements for development:</p>
<div class="highlight highlight-source-shell"><pre>sudo pacman -S clang</pre></div>
<h3><a id="user-content-windowsmac" class="anchor" aria-hidden="true" href="#windowsmac"><svg class="octicon octicon-link" viewBox="0 0 16 16" version="1.1" width="16" height="16" aria-hidden="true"><path fill-rule="evenodd" d="M7.775 3.275a.75.75 0 001.06 1.06l1.25-1.25a2 2 0 112.83 2.83l-2.5 2.5a2 2 0 01-2.83 0 .75.75 0 00-1.06 1.06 3.5 3.5 0 004.95 0l2.5-2.5a3.5 3.5 0 00-4.95-4.95l-1.25 1.25zm-4.69 9.64a2 2 0 010-2.83l2.5-2.5a2 2 0 012.83 0 .75.75 0 001.06-1.06 3.5 3.5 0 00-4.95 0l-2.5 2.5a3.5 3.5 0 004.95 4.95l1.25-1.25a.75.75 0 00-1.06-1.06l-1.25 1.25a2 2 0 01-2.83 0z"></path></svg></a>Windows/Mac</h3>
<p>¯\(°_o)/¯</p>
<p>....install linux?</p>
<p>(Have a look at the <a href="/alxhoff/FreeRTOS-Emulator/wiki/Remote-Toolchain">Remote Toolchain</a> wiki section)</p>
<h2><a id="user-content-building" class="anchor" aria-hidden="true" href="#building"><svg class="octicon octicon-link" viewBox="0 0 16 16" version="1.1" width="16" height="16" aria-hidden="true"><path fill-rule="evenodd" d="M7.775 3.275a.75.75 0 001.06 1.06l1.25-1.25a2 2 0 112.83 2.83l-2.5 2.5a2 2 0 01-2.83 0 .75.75 0 00-1.06 1.06 3.5 3.5 0 004.95 0l2.5-2.5a3.5 3.5 0 00-4.95-4.95l-1.25 1.25zm-4.69 9.64a2 2 0 010-2.83l2.5-2.5a2 2 0 012.83 0 .75.75 0 001.06-1.06 3.5 3.5 0 00-4.95 0l-2.5 2.5a3.5 3.5 0 004.95 4.95l1.25-1.25a.75.75 0 00-1.06-1.06l-1.25 1.25a2 2 0 01-2.83 0z"></path></svg></a>Building</h2>
<div class="highlight highlight-source-shell"><pre><span class="pl-c1">cd</span> build
cmake ..
make</pre></div>
<p>For those requiring an IDE run</p>
<div class="highlight highlight-source-shell"><pre>cmake -G <span class="pl-s"><span class="pl-pds">"</span>Eclipse CDT4 - Unix Makefiles<span class="pl-pds">"</span></span> ./</pre></div>
<p>to generate the appropriate project files to allow for the emulator to be imported into Eclipse.</p>
<h3><a id="user-content-additional-targets" class="anchor" aria-hidden="true" href="#additional-targets"><svg class="octicon octicon-link" viewBox="0 0 16 16" version="1.1" width="16" height="16" aria-hidden="true"><path fill-rule="evenodd" d="M7.775 3.275a.75.75 0 001.06 1.06l1.25-1.25a2 2 0 112.83 2.83l-2.5 2.5a2 2 0 01-2.83 0 .75.75 0 00-1.06 1.06 3.5 3.5 0 004.95 0l2.5-2.5a3.5 3.5 0 00-4.95-4.95l-1.25 1.25zm-4.69 9.64a2 2 0 010-2.83l2.5-2.5a2 2 0 012.83 0 .75.75 0 001.06-1.06 3.5 3.5 0 00-4.95 0l-2.5 2.5a3.5 3.5 0 004.95 4.95l1.25-1.25a.75.75 0 00-1.06-1.06l-1.25 1.25a2 2 0 01-2.83 0z"></path></svg></a>Additional targets</h3>
<h4><a id="user-content-documentation" class="anchor" aria-hidden="true" href="#documentation"><svg class="octicon octicon-link" viewBox="0 0 16 16" version="1.1" width="16" height="16" aria-hidden="true"><path fill-rule="evenodd" d="M7.775 3.275a.75.75 0 001.06 1.06l1.25-1.25a2 2 0 112.83 2.83l-2.5 2.5a2 2 0 01-2.83 0 .75.75 0 00-1.06 1.06 3.5 3.5 0 004.95 0l2.5-2.5a3.5 3.5 0 00-4.95-4.95l-1.25 1.25zm-4.69 9.64a2 2 0 010-2.83l2.5-2.5a2 2 0 012.83 0 .75.75 0 001.06-1.06 3.5 3.5 0 00-4.95 0l-2.5 2.5a3.5 3.5 0 004.95 4.95l1.25-1.25a.75.75 0 00-1.06-1.06l-1.25 1.25a2 2 0 01-2.83 0z"></path></svg></a>Documentation</h4>
<p>Doxygen documentation, found in the <a href="/alxhoff/FreeRTOS-Emulator/blob/master/docs">docs</a> folder, can be generated from cmake/make by passing the variable <code>DOCS=on</code> and making the target <code>docs</code>.</p>
<div class="highlight highlight-source-shell"><pre>cmake -DDOCS=on ..
make docs</pre></div>
<h4><a id="user-content-tests" class="anchor" aria-hidden="true" href="#tests"><svg class="octicon octicon-link" viewBox="0 0 16 16" version="1.1" width="16" height="16" aria-hidden="true"><path fill-rule="evenodd" d="M7.775 3.275a.75.75 0 001.06 1.06l1.25-1.25a2 2 0 112.83 2.83l-2.5 2.5a2 2 0 01-2.83 0 .75.75 0 00-1.06 1.06 3.5 3.5 0 004.95 0l2.5-2.5a3.5 3.5 0 00-4.95-4.95l-1.25 1.25zm-4.69 9.64a2 2 0 010-2.83l2.5-2.5a2 2 0 012.83 0 .75.75 0 001.06-1.06 3.5 3.5 0 00-4.95 0l-2.5 2.5a3.5 3.5 0 004.95 4.95l1.25-1.25a.75.75 0 00-1.06-1.06l-1.25 1.25a2 2 0 01-2.83 0z"></path></svg></a>Tests</h4>
<p>In <a href="/alxhoff/FreeRTOS-Emulator/blob/master/cmake/test.cmake"><code>test.cmake</code></a> a number of extra targets are provided to help with linting.</p>
<h4><a id="user-content-git---check" class="anchor" aria-hidden="true" href="#git---check"><svg class="octicon octicon-link" viewBox="0 0 16 16" version="1.1" width="16" height="16" aria-hidden="true"><path fill-rule="evenodd" d="M7.775 3.275a.75.75 0 001.06 1.06l1.25-1.25a2 2 0 112.83 2.83l-2.5 2.5a2 2 0 01-2.83 0 .75.75 0 00-1.06 1.06 3.5 3.5 0 004.95 0l2.5-2.5a3.5 3.5 0 00-4.95-4.95l-1.25 1.25zm-4.69 9.64a2 2 0 010-2.83l2.5-2.5a2 2 0 012.83 0 .75.75 0 001.06-1.06 3.5 3.5 0 00-4.95 0l-2.5 2.5a3.5 3.5 0 004.95 4.95l1.25-1.25a.75.75 0 00-1.06-1.06l-1.25 1.25a2 2 0 01-2.83 0z"></path></svg></a>Git --check</h4>
<div class="highlight highlight-source-shell"><pre>make commit</pre></div>
<p>Checks for whitespaces and empty lines.</p>
<h4><a id="user-content-astyle-formatting" class="anchor" aria-hidden="true" href="#astyle-formatting"><svg class="octicon octicon-link" viewBox="0 0 16 16" version="1.1" width="16" height="16" aria-hidden="true"><path fill-rule="evenodd" d="M7.775 3.275a.75.75 0 001.06 1.06l1.25-1.25a2 2 0 112.83 2.83l-2.5 2.5a2 2 0 01-2.83 0 .75.75 0 00-1.06 1.06 3.5 3.5 0 004.95 0l2.5-2.5a3.5 3.5 0 00-4.95-4.95l-1.25 1.25zm-4.69 9.64a2 2 0 010-2.83l2.5-2.5a2 2 0 012.83 0 .75.75 0 001.06-1.06 3.5 3.5 0 00-4.95 0l-2.5 2.5a3.5 3.5 0 004.95 4.95l1.25-1.25a.75.75 0 00-1.06-1.06l-1.25 1.25a2 2 0 01-2.83 0z"></path></svg></a>Astyle Formatting</h4>
<div class="highlight highlight-source-shell"><pre>cmake -DENABLE_ASTYLE=ON ..
make format</pre></div>
<p>Invokes the Astyle formatter.</p>
<p><strong>Warning:</strong> The default version of CMake which is installed f.e. on Ubuntu 16.04 will throw an arror when running make to setup the <code>bin/astyle</code> binary. Please upgrade to a newrer version manually if required:</p>
<div class="highlight highlight-source-shell"><pre>VERSION=3.16
BUILD=5
wget -q https://cmake.org/files/v<span class="pl-smi">$VERSION</span>/cmake-<span class="pl-smi">$VERSION</span>.<span class="pl-smi">$BUILD</span>-Linux-x86_64.sh
mkdir /opt/cmake
sh cmake-<span class="pl-smi">$VERSION</span>.<span class="pl-smi">$BUILD</span>-Linux-x86_64.sh --prefix=/opt/cmake --skip-license --exclude-subdir
ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake
rm cmake-<span class="pl-smi">$VERSION</span>.<span class="pl-smi">$BUILD</span>-Linux-x86_64.sh</pre></div>
<h4><a id="user-content-clang-tidy" class="anchor" aria-hidden="true" href="#clang-tidy"><svg class="octicon octicon-link" viewBox="0 0 16 16" version="1.1" width="16" height="16" aria-hidden="true"><path fill-rule="evenodd" d="M7.775 3.275a.75.75 0 001.06 1.06l1.25-1.25a2 2 0 112.83 2.83l-2.5 2.5a2 2 0 01-2.83 0 .75.75 0 00-1.06 1.06 3.5 3.5 0 004.95 0l2.5-2.5a3.5 3.5 0 00-4.95-4.95l-1.25 1.25zm-4.69 9.64a2 2 0 010-2.83l2.5-2.5a2 2 0 012.83 0 .75.75 0 001.06-1.06 3.5 3.5 0 00-4.95 0l-2.5 2.5a3.5 3.5 0 004.95 4.95l1.25-1.25a.75.75 0 00-1.06-1.06l-1.25 1.25a2 2 0 01-2.83 0z"></path></svg></a>Clang Tidy</h4>
<div class="highlight highlight-source-shell"><pre>cmake -DENABLE_CLANG_TIDY=ON ..
make tidy</pre></div>
<p>Uses clang tidy to find style violations, interface misuse of bugs found via static analysis.</p>
<p>To generate a list of warnings/errors use the build target <code>tidy_list</code> and then view the file <code>tidy.fixes</code>.</p>
<h4><a id="user-content-cppcheck" class="anchor" aria-hidden="true" href="#cppcheck"><svg class="octicon octicon-link" viewBox="0 0 16 16" version="1.1" width="16" height="16" aria-hidden="true"><path fill-rule="evenodd" d="M7.775 3.275a.75.75 0 001.06 1.06l1.25-1.25a2 2 0 112.83 2.83l-2.5 2.5a2 2 0 01-2.83 0 .75.75 0 00-1.06 1.06 3.5 3.5 0 004.95 0l2.5-2.5a3.5 3.5 0 00-4.95-4.95l-1.25 1.25zm-4.69 9.64a2 2 0 010-2.83l2.5-2.5a2 2 0 012.83 0 .75.75 0 001.06-1.06 3.5 3.5 0 00-4.95 0l-2.5 2.5a3.5 3.5 0 004.95 4.95l1.25-1.25a.75.75 0 00-1.06-1.06l-1.25 1.25a2 2 0 01-2.83 0z"></path></svg></a>CppCheck</h4>
<div class="highlight highlight-source-shell"><pre>cmake -DENABLE_CPPCHECK=ON ..
make check</pre></div>
<p>Code analysis with CppCheck, focusing on undefined behaviour bugs.</p>
<h4><a id="user-content-valgrind-memcheck" class="anchor" aria-hidden="true" href="#valgrind-memcheck"><svg class="octicon octicon-link" viewBox="0 0 16 16" version="1.1" width="16" height="16" aria-hidden="true"><path fill-rule="evenodd" d="M7.775 3.275a.75.75 0 001.06 1.06l1.25-1.25a2 2 0 112.83 2.83l-2.5 2.5a2 2 0 01-2.83 0 .75.75 0 00-1.06 1.06 3.5 3.5 0 004.95 0l2.5-2.5a3.5 3.5 0 00-4.95-4.95l-1.25 1.25zm-4.69 9.64a2 2 0 010-2.83l2.5-2.5a2 2 0 012.83 0 .75.75 0 001.06-1.06 3.5 3.5 0 00-4.95 0l-2.5 2.5a3.5 3.5 0 004.95 4.95l1.25-1.25a.75.75 0 00-1.06-1.06l-1.25 1.25a2 2 0 01-2.83 0z"></path></svg></a>Valgrind (memcheck)</h4>
<div class="highlight highlight-source-shell"><pre>cmake -DENABLE_MEMCHECK=ON ..
make memcheck</pre></div>
<p>Memory checker.</p>
<h4><a id="user-content-google-testscoverage" class="anchor" aria-hidden="true" href="#google-testscoverage"><svg class="octicon octicon-link" viewBox="0 0 16 16" version="1.1" width="16" height="16" aria-hidden="true"><path fill-rule="evenodd" d="M7.775 3.275a.75.75 0 001.06 1.06l1.25-1.25a2 2 0 112.83 2.83l-2.5 2.5a2 2 0 01-2.83 0 .75.75 0 00-1.06 1.06 3.5 3.5 0 004.95 0l2.5-2.5a3.5 3.5 0 00-4.95-4.95l-1.25 1.25zm-4.69 9.64a2 2 0 010-2.83l2.5-2.5a2 2 0 012.83 0 .75.75 0 001.06-1.06 3.5 3.5 0 00-4.95 0l-2.5 2.5a3.5 3.5 0 004.95 4.95l1.25-1.25a.75.75 0 00-1.06-1.06l-1.25 1.25a2 2 0 01-2.83 0z"></path></svg></a>Google Tests/Coverage</h4>
<p>Coverage</p>
<div class="highlight highlight-source-shell"><pre>cmake -DENABLE_COVERAGE=ON ..
make</pre></div>
<p>Each sanitizer must be run stand alone, thus you cannot run them together.</p>
<p>Address sanitizer</p>
<div class="highlight highlight-source-shell"><pre>cmake -DENABLE_ASAN=ON ..
make</pre></div>
<p>Undefined behaviour sanitizer</p>
<div class="highlight highlight-source-shell"><pre>cmake -DENABLE_USAN=ON ..
make</pre></div>
<p>Thread sanitizer</p>
<div class="highlight highlight-source-shell"><pre>cmake -DENABLE_TSAN=ON ..
make</pre></div>
<h3><a id="user-content-all-checks" class="anchor" aria-hidden="true" href="#all-checks"><svg class="octicon octicon-link" viewBox="0 0 16 16" version="1.1" width="16" height="16" aria-hidden="true"><path fill-rule="evenodd" d="M7.775 3.275a.75.75 0 001.06 1.06l1.25-1.25a2 2 0 112.83 2.83l-2.5 2.5a2 2 0 01-2.83 0 .75.75 0 00-1.06 1.06 3.5 3.5 0 004.95 0l2.5-2.5a3.5 3.5 0 00-4.95-4.95l-1.25 1.25zm-4.69 9.64a2 2 0 010-2.83l2.5-2.5a2 2 0 012.83 0 .75.75 0 001.06-1.06 3.5 3.5 0 00-4.95 0l-2.5 2.5a3.5 3.5 0 004.95 4.95l1.25-1.25a.75.75 0 00-1.06-1.06l-1.25 1.25a2 2 0 01-2.83 0z"></path></svg></a>All checks</h3>
<p>The target <code>make all_checks</code></p>
<div class="highlight highlight-source-shell"><pre>cmake -DALL_CHECKS=ON ..
make</pre></div>
<p>will perform all checks</p>
<h2><a id="user-content-running" class="anchor" aria-hidden="true" href="#running"><svg class="octicon octicon-link" viewBox="0 0 16 16" version="1.1" width="16" height="16" aria-hidden="true"><path fill-rule="evenodd" d="M7.775 3.275a.75.75 0 001.06 1.06l1.25-1.25a2 2 0 112.83 2.83l-2.5 2.5a2 2 0 01-2.83 0 .75.75 0 00-1.06 1.06 3.5 3.5 0 004.95 0l2.5-2.5a3.5 3.5 0 00-4.95-4.95l-1.25 1.25zm-4.69 9.64a2 2 0 010-2.83l2.5-2.5a2 2 0 012.83 0 .75.75 0 001.06-1.06 3.5 3.5 0 00-4.95 0l-2.5 2.5a3.5 3.5 0 004.95 4.95l1.25-1.25a.75.75 0 00-1.06-1.06l-1.25 1.25a2 2 0 01-2.83 0z"></path></svg></a>Running</h2>
<p>The binary will be created inside a <code>bin</code> folder. The emulator should be run from this folder becuase at the moment the Gfx libraries rely on hardcoded resource paths for things such as fonts. As such to run perform the following.</p>
<div class="highlight highlight-source-shell"><pre><span class="pl-c1">cd</span> bin
./FreeRTOS_Emulator</pre></div>
<h2><a id="user-content-debugging" class="anchor" aria-hidden="true" href="#debugging"><svg class="octicon octicon-link" viewBox="0 0 16 16" version="1.1" width="16" height="16" aria-hidden="true"><path fill-rule="evenodd" d="M7.775 3.275a.75.75 0 001.06 1.06l1.25-1.25a2 2 0 112.83 2.83l-2.5 2.5a2 2 0 01-2.83 0 .75.75 0 00-1.06 1.06 3.5 3.5 0 004.95 0l2.5-2.5a3.5 3.5 0 00-4.95-4.95l-1.25 1.25zm-4.69 9.64a2 2 0 010-2.83l2.5-2.5a2 2 0 012.83 0 .75.75 0 001.06-1.06 3.5 3.5 0 00-4.95 0l-2.5 2.5a3.5 3.5 0 004.95 4.95l1.25-1.25a.75.75 0 00-1.06-1.06l-1.25 1.25a2 2 0 01-2.83 0z"></path></svg></a>Debugging</h2>
<p>The emulator uses the signals <code>SIGUSR1</code> and <code>SIG34</code> and as such GDB needs to be told to ignore the signal.
An appropriate <code>.gdbinit</code> is in the <code>bin</code> directory.
Copy the <code>.gdbinit</code> into your home directory or make sure to debug from the <code>bin</code> directory.
Such that GDB does not get interrupted by the POSIX signals used by the emulator for IPC.</p>
<p>If using an IDE, make sure to configure your debug to load the gdbinit file.</p>
<h2><a id="user-content-tracing" class="anchor" aria-hidden="true" href="#tracing"><svg class="octicon octicon-link" viewBox="0 0 16 16" version="1.1" width="16" height="16" aria-hidden="true"><path fill-rule="evenodd" d="M7.775 3.275a.75.75 0 001.06 1.06l1.25-1.25a2 2 0 112.83 2.83l-2.5 2.5a2 2 0 01-2.83 0 .75.75 0 00-1.06 1.06 3.5 3.5 0 004.95 0l2.5-2.5a3.5 3.5 0 00-4.95-4.95l-1.25 1.25zm-4.69 9.64a2 2 0 010-2.83l2.5-2.5a2 2 0 012.83 0 .75.75 0 001.06-1.06 3.5 3.5 0 00-4.95 0l-2.5 2.5a3.5 3.5 0 004.95 4.95l1.25-1.25a.75.75 0 00-1.06-1.06l-1.25 1.25a2 2 0 01-2.83 0z"></path></svg></a>Tracing</h2>
<p><em>Note: this is experiemental and proves to be unstable with the AIO libraries, it was used during development of the emulator and provides a novel function for small experiements, it should not be used for serious debugging of the entire emulator as this will cause errors.</em></p>
<p>Tracing, found in <a href="/alxhoff/FreeRTOS-Emulator/blob/master/lib/tracer">lib/tracer</a> is instrumented using GCC's function instrumentation.</p>
<p>Running</p>
<div class="highlight highlight-source-shell"><pre>cmake -DTRACE_FUNCTIONS=ON ..</pre></div>
<p>will include the constructor, destructor and the needed <code>__cyg_profile_func_xxx</code> functions that are called upon entry and exit of any functions called during the execution of the program.
These callback functions are implemented to log the function, caller and timestamp of each function calls, written to a file named <code>trace.out</code>.
As the values written out are memory offsets and, as such, are not human readable the trace dump must be processed by the script <a href="/alxhoff/FreeRTOS-Emulator/blob/master/lib/tracer/readtracelog.sh"><code>readtracelog.sh</code></a> which uses <a href="https://linux.die.net/man/1/addr2line" rel="nofollow"><code>addr2line</code></a> to convert the memory offsets into human readable function calls, done using the memory map of the compiled executable.</p>
<h3><a id="user-content-example" class="anchor" aria-hidden="true" href="#example"><svg class="octicon octicon-link" viewBox="0 0 16 16" version="1.1" width="16" height="16" aria-hidden="true"><path fill-rule="evenodd" d="M7.775 3.275a.75.75 0 001.06 1.06l1.25-1.25a2 2 0 112.83 2.83l-2.5 2.5a2 2 0 01-2.83 0 .75.75 0 00-1.06 1.06 3.5 3.5 0 004.95 0l2.5-2.5a3.5 3.5 0 00-4.95-4.95l-1.25 1.25zm-4.69 9.64a2 2 0 010-2.83l2.5-2.5a2 2 0 012.83 0 .75.75 0 001.06-1.06 3.5 3.5 0 00-4.95 0l-2.5 2.5a3.5 3.5 0 004.95 4.95l1.25-1.25a.75.75 0 00-1.06-1.06l-1.25 1.25a2 2 0 01-2.83 0z"></path></svg></a>Example</h3>
<p>Adding something like</p>
<div class="highlight highlight-source-shell"><pre>+void printhello(void)
+{
+    printf(<span class="pl-s"><span class="pl-pds">"</span>hello<span class="pl-pds">"</span></span>)<span class="pl-k">;</span>
+}
+
 int main(int argc, char <span class="pl-k">*</span>argv[])
 {
+    <span class="pl-en">printhello</span>();</pre></div>
<p>To your code and then compiling and running the executable, after passing <code>-DTRACE_FUNCTIONS=ON</code> to cmake of course, you will be presented with an output similar to</p>
<div class="highlight highlight-source-shell"><pre>x 0x55e138f918a9 0x55e138f9fe4d 1587039262
e 0x55e138f92f72 0x7f2117466023 1587039262
e 0x55e138f92f34 0x55e138f92f99 1587039262
x 0x55e138f92f34 0x55e138f92f99 1587039262
e 0x55e138f918f0 0x7f2117bb242b 1587039262</pre></div>
<p>After processing using the provided script sorcery, running something such as</p>
<div class="highlight highlight-source-shell"><pre>./readtracelog.sh ../../bin/FreeRTOS_Emulator ../../build/trace.out</pre></div>
<p>You will see a human readable output that logs the function entries and exit made in the program</p>
<div class="highlight highlight-source-shell"><pre>Exit  trace_begin at 2020-04-16T14:14:22+02:00
Enter main at 2020-04-16T14:14:22+02:00, called from <span class="pl-k">??</span> (<span class="pl-k">??</span>:0)
Enter printhello at 2020-04-16T14:14:22+02:00, called from main (main.c:624)
Exit  printhello at 2020-04-16T14:14:22+02:00
Enter trace_end at 2020-04-16T14:14:22+02:00, called from <span class="pl-k">??</span> (<span class="pl-k">??</span>:0)</pre></div>
<p>Note that the ?? visible in the output above are the result of the function instrumentation only being able to map to functions compiled using the <code>-finstrument-functions</code> compile flag.
Extenal libraries etc are only linked against and not compiled using this flag, therefore they cannot be instrumented.</p>
<hr>
</body>
</html>

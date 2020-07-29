Replayer
========

### Traces definition file

The trace definition file contains information about the traces to run along
with their expected image checksums on each device, and optionally from where to
download them. An example:

```yaml
traces-db:
  download-url: https://minio-packet.freedesktop.org/mesa-tracie-public/

traces:
  - path: glmark2/jellyfish.rdc
    expectations:
      - device: gl-intel-0x3185
        checksum: 58359ea4caf6ad44c6b65526881bbd17
      - device: gl-vmware-llvmpipe
        checksum: d82267c25a0decdad7b563c56bb81106
  - path: supertuxkart/supertuxkart-antediluvian-abyss.rdc
    expectations:
      - device: gl-intel-0x3185
        checksum: ff827f7eb069afd87cc305a422cba939
```

The `traces-db` entry can be absent, in which case it is assumed that
the traces can be found in the `CWD/replayer-db` directory.

Traces that don't have an expectation for the current device are skipped
during trace replay.

Adding a new trace to the list involves uploading the trace to the
remote `traces-db` and adding an entry to the `traces` list. The
reference checksums can be calculated with the `checksum` command.
Alternatively, an arbitrary checksum can be used, and during replay
(see below) the scripts will report the mismatch and expected
checksum.

### Trace-db download urls

The trace-db:download-url property contains an HTTPS url from which traces can
be downloaded, by appending traces:path properties to it.

### Trace files

replayer supports renderdoc (.rdc), apitrace (.trace, .trace-dxgi) and
gfxreconstruct (.gfxr) files. Trace files need to have the correct
extension so that replayer can detect them properly.

The trace files that are contained in public `traces-db` stores must
be legally redistributable. This is typically true for FOSS games and
applications. Traces for proprietary games and application are
typically not redistributable, unless specific redistribution rights
have been granted by the publisher.

In order to have reliable comparisons, trace files in a given store
are expected to be immutable. Any change to a trace file means that it
needs to be renamed and updated in the traces definition file (eg. by
appending a _v2 suffix to the file).

### Replaying traces

replayer features a series of commands to deal with traces:
 * `checksum`: will calculate the checksum for a given image file.
 * `compare`: will download a trace or all the traces in a traces
   definition file for a given device, replay them and compare their
   checksum against the expected ones.
 * `download`: will download a file, given a relative path, from a
   remote url.
 * `dump`: will dump as images the last call or specified calls from a
   trace file, given a specific device.
 * `query`: will return the queried information from a given traces
   definition file.

You can get a more complete help running:

   ```sh
   $ replayer.py <command> --help
   ```

Examples:

   ```sh
   $ replayer.py checksum ./vkcube.gfxr-9.png
   ```

   ```sh
   $ replayer.py compare trace \
                 --output ./results \
		 --device-name gl-vmware-llvmpipe \
		 --download-url https://minio-packet.freedesktop.org/mesa-tracie-public/ \
		 glmark2/desktop-blur-radius=5:effect=blur:passes=1:separable=true:windows=4.rdc \
		 8867f3a41f180626d0d4b7661ff5c0f4
   ```

   ```sh
   $ replayer.py compare yaml \
		 --yaml-file ./traces.yml \
		 --device-name gl-vmware-llvmpipe \
		 --keep-image
   ```

   ```sh
   $ replayer.py download \
		 --download-url https://minio-packet.freedesktop.org/mesa-tracie-public/ \
		 --db-path ./traces-db \
		 --force-download \
		 glmark2/desktop-blur-radius=5:effect=blur:passes=1:separable=true:windows=4.rdc
   ```

   ```sh
   $ replayer.py dump \
                 --config ./piglit.conf \
                 --output ./results \
		 --device-name gl-vmware-llvmpipe \
		 --calls "3,8,9" \
		 glmark2/desktop-blur-radius=5:effect=blur:passes=1:separable=true:windows=4.rdc
   ```

   ```sh
   $ replayer.py query \
		 --yaml-file ./traces.yml \
                 checksum \
                 --device-name gl-vmware-llvmpipe \
		 glmark2/desktop-blur-radius=5:effect=blur:passes=1:separable=true:windows=4.rdc
   ```

   ```sh
   $ replayer.py query \
		 --yaml-file ./traces.yml \
                 traces \
                 --device-name gl-vmware-llvmpipe \
                 --trace-types "apitrace,renderdoc"
                 --checksum
   ```

   ```sh
   $ replayer.py query \
		 --yaml-file ./traces.yml \
                 traces_db_download_url
   ```

Unless specified when comparing or dumping, replayer places the
produced artifacts at the `CWD/results` directory. By default, created
images from traces are only stored in case of a checksum
mismatch. This can be overriden with the `--keep-image` parameter to
force storing images, e.g., to get a complete set of reference images.

By default when dumping, only the image corresponding to the last frame
of the trace is created.  This can be changed with the `--calls`
parameter.

Unless specified with the `--output` parameter, the dumped images are
stored in the subdirectory `./test/<device-name>/<trace_file_path/` ,
with names of the form `trace_file_name-call_num.png`.  The full log
of any commands used while dumping the images is also saved in a file
in the 'test/<device-name>' subdirectory, named after the trace name
with '.log' appended.

### Specific dependencies for dumping depending of the trace type

Depending on the target 3D API, replayer requires a recent version
of apitrace (and eglretrace) being in the path, and also the renderdoc
python module being available, for GL traces.

To ensure python3 can find the renderdoc python module you need to set
`PYTHONPATH` to point to the location of `renderdoc.so` (binary python modules)
and `LD_LIBRARY_PATH` to point to the location of `librenderdoc.so`. In the
renderdoc build tree, both of these are in `renderdoc/<builddir>/lib`. Note
that renderdoc doesn't install the `renderdoc.so` python module.

In the case of Vulkan traces, replayer needs a recent version of
gfxrecon-info and gfxrecon-replay being in the path, and also the
`VK_LAYER_LUNARG_screenshot` Vulkan layer from LunarG's VulkanTools.

To ensure that this layer can be found when running the trace you need
to set `VK_LAYER_PATH` to point to the location of
`VkLayer_screenshot.json` and `LD_LIBRARY_PATH` to point to the
location of `libVkLayer_screenshot.so`.

In the case of DXGI traces, replayer requires Wine, a recent version
of DXVK installed in the default `WINEPREFIX`, and a recent binary
version of apitrace (and d3dretrace) for Windows which should be
reachable through Windows' `PATH` environment variable.

Alternatively, all of the paths for specific binaries can be set via
piglit's configuration file or env variables. Check the documenation
at [piglit.conf.example](piglit.conf.example) for further details.

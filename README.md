# Mask Bass

Plugin de sidechain/volume-shaper (estilo Kickstart) para Windows. VST3 (y AAX opcional para Pro Tools).

## Funciones
- **Rate**: 1/8, 1/4, 1/2, 1/1 (ciclo de la curva)
- **Trigger**: SYNC (tempo del host), MIDI (note-on), AUDIO (sidechain externo + umbral)
- **16 curvas** de ganancia, flechas para navegar
- **MIX** (profundidad del ducking)
- **BAND**: aplica el efecto solo por debajo de la frecuencia de crossover (Linkwitz-Riley); auriculares = escuchar solo esa banda

## Compilar
Automatico: cada `push` ejecuta GitHub Actions y deja el `.vst3` en *Actions > ultimo run > Artifacts*.

Local (Windows, Visual Studio 2022 + CMake):
```
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```
Resultado: `build/MaskBass_artefacts/Release/VST3/Mask Bass.vst3` -> copiar a `C:\Program Files\Common Files\VST3`.

## Pro Tools (AAX)
Pro Tools NO carga VST3. Necesitas el AAX SDK de Avid (gratis con cuenta de desarrollador):
```
cmake -B build -G "Visual Studio 17 2022" -A x64 -DAAX_SDK_PATH="C:/SDKs/AAX_SDK"
cmake --build build --config Release
```
Copiar el `.aaxplugin` a `C:\Program Files\Common Files\Avid\Audio\Plug-Ins`.
Las versiones normales de Pro Tools solo cargan AAX firmados con PACE/iLok (wraptool); las builds de desarrollo cargan AAX sin firmar.

## Licencia
Usa JUCE: AGPLv3 (si publicas el codigo) o licencia comercial de JUCE.

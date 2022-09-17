# WNF

Essentially this is a C port of the C# project https://github.com/ustayready/CasperStager

## Solution Structure

The solution is composed of two projects
- wnf: The main WNF API code
- payload: The mock payload code

### wnf
This project reads the payload into a buffer, saves off the payload into three different WNF states in 0x1000 byte chunks. And then retrieves the chunked data from the WNF states and recombines the chunks into the full payload. This payload is then written to disk and can be run.

### payload
This project provides a mock payload to illustrate how an EXE can be saved off into WNF states as chunks, and then can be pieced back together.

## Building
Built with VS2019 16.11.16 Community

## Running

As is, `wnf.exe` expects the `Release` version of `payload.exe` to be at `C:\payload.exe`. This is because the release payload is < 12k bytes and so it will fit in 3 WNF states.

`wnf.exe` must be run with at least `admin` rights.

## WNF States
The WNF states chosen worked on my machine (Win10 21H2), but some can be OS-version dependent. I've found that the XBOX WNF states work best.

A long list of known WNF states can be found at https://github.com/googleprojectzero/sandbox-attacksurface-analysis-tools/blob/main/NtApiDotNet/NtWnfWellKnownNames.cs#L40

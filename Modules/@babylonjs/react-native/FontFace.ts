import { Tools } from '@babylonjs/core';

// Declare _native to get access to NativeCanvas.
declare var _native: any;

/**
 * Partial Polyfill for FontFace Web API to wrap the NativeCanvas object.
 */
export class FontFace {
  private source: string | ArrayBuffer | undefined;
  private status: "unloaded" | "loading" | "loaded" | "error" = "unloaded";
  public constructor(public readonly family: string, source: string | ArrayBuffer) {
    this.source = source;
  }

  public async load(): Promise<void> {
    try {
      this.status = "loading";
      if (typeof this.source === 'string') {
        this.source = await Tools.LoadFileAsync(this.source as string);
      }

      await _native.NativeCanvas.loadTTFAsync(this.family, this.source);
      this.source = undefined;
      this.status = "loaded"
    } catch (ex) {
      console.error("Error encountered when loading font: " + ex);
      this.status = "error";
    }
  }

  public get loaded(): boolean {
    return this.status === "loaded";
  }
}
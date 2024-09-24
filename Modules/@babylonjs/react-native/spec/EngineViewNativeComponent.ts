import type { HostComponent, ViewProps } from 'react-native';
import codegenNativeCommands from 'react-native/Libraries/Utilities/codegenNativeCommands';
import codegenNativeComponent from 'react-native/Libraries/Utilities/codegenNativeComponent';

import type {
  DirectEventHandler,
  WithDefault,
  Int32
} from 'react-native/Libraries/Types/CodegenTypes';

export interface NativeProps extends ViewProps {
  isTransparent?: WithDefault<boolean, false>;
  antiAliasing?: Int32;
  onSnapshotDataReturned?: DirectEventHandler<null>;
  // Android only
  androidView?: string;
}

type EngineViewViewType = HostComponent<NativeProps>;

export interface NativeCommands {
  takeSnapshot: (viewRef: React.ElementRef<EngineViewViewType>) => Promise<string>;
}

export const Commands: NativeCommands = codegenNativeCommands<NativeCommands>({
  supportedCommands: [
    'takeSnapshot'
  ],
});

export default codegenNativeComponent<NativeProps>(
  'EngineViewNativeComponent'
) as HostComponent<NativeProps>;

/**
 * Generated with the TypeScript template
 * https://github.com/react-native-community/react-native-template-typescript
 *
 * @format
 */

import React, { useState, FunctionComponent, useEffect, useCallback } from 'react';
import { SafeAreaView, StatusBar, Button, View, Text, ViewProps, Image, NativeModules } from 'react-native';

import { EngineView, useEngine, EngineViewCallbacks } from '@babylonjs/react-native';
import { Scene, Vector3, Mesh, ArcRotateCamera, Camera, PBRMetallicRoughnessMaterial, Color3, TargetCamera, WebXRSessionManager, Engine, Color4 } from '@babylonjs/core';
// import Slider from '@react-native-community/slider';

console.log("Setting up EngineScreen");
const EngineScreen: FunctionComponent<ViewProps> = (props: ViewProps) => {
  const defaultScale = 1;
  const enableSnapshots = false;

  console.log("Calling EngineScreen useEngine");
  const engine = useEngine();

  console.log("Calling EngineScreen useState instances");
  const [toggleView, setToggleView] = useState(false);
  const [camera, setCamera] = useState<Camera>();
  const [box, setBox] = useState<Mesh>();
  const [scene, setScene] = useState<Scene>();
  const [xrSession, setXrSession] = useState<WebXRSessionManager>();
  const [scale, setScale] = useState<number>(defaultScale);
  const [snapshotData, setSnapshotData] = useState<string>();
  // const [engineViewCallbacks, setEngineViewCallbacks] = useState<EngineViewCallbacks>();

  console.log("Declaring EngineScreen useEffect instances");
  useEffect(() => {
    console.log("useEffect called to setup scene");
    if (engine) {
      console.log("Engine populated");
      const scene = new Scene(engine);
      setScene(scene);
      console.log("Scene created");
      scene.createDefaultCamera(true);
      (scene.activeCamera as ArcRotateCamera).beta -= Math.PI / 8;
      setCamera(scene.activeCamera!);
      console.log("Camera created");
      scene.createDefaultLight(true);
      console.log("Default content created");

      const box = Mesh.CreateBox("box", 0.3, scene);
      setBox(box);
      const mat = new PBRMetallicRoughnessMaterial("mat", scene);
      mat.metallic = 1;
      mat.roughness = 0.5;
      mat.baseColor = Color3.Red();
      box.material = mat;
      console.log("Box created");

      scene.beforeRender = function () {
        console.log("updating box rotation");
        box.rotate(Vector3.Up(), 0.005 * scene.getAnimationRatio());
      };
    }
  }, [engine]);

  console.log("Declaring EngineScreen box scale change");
  useEffect(() => {
    if (box) {
      console.log("updating box scale");
      box.scaling = new Vector3(scale, scale, scale);
    }
  }, [box, scale]);

  console.log("Declaring onToggleXr")
  const onToggleXr = useCallback(() => {
    console.log("Calling onToggleXr");
    (async () => {
      if (xrSession) {
        await xrSession.exitXRAsync();
        setXrSession(undefined);
      } else {
        if (box !== undefined && scene !== undefined) {
          const xr = await scene.createDefaultXRExperienceAsync({ disableDefaultUI: true, disableTeleportation: true })
          const session = await xr.baseExperience.enterXRAsync("immersive-ar", "unbounded", xr.renderTarget);
          setXrSession(session);

          // TODO: Figure out why getFrontPosition stopped working
          //box.position = (scene.activeCamera as TargetCamera).getFrontPosition(2);
          const cameraRay = scene.activeCamera!.getForwardRay(1);
          box.position = cameraRay.origin.add(cameraRay.direction.scale(cameraRay.length));
          box.rotate(Vector3.Up(), 3.14159);

          // needed for hmds
          scene.autoClear = true;
          scene.clearColor = new Color4(0, 0, 0, 0);
        }
      }
    })();
  }, [box, scene, xrSession]);

  // const onInitialized = useCallback(async(engineViewCallbacks: EngineViewCallbacks) => {
  //   setEngineViewCallbacks(engineViewCallbacks);
  // }, [engine]);

  // const onSnapshot = useCallback(async () => {
  //   if (engineViewCallbacks) {
  //     setSnapshotData("data:image/jpeg;base64," + await engineViewCallbacks.takeSnapshot());
  //   }
  // }, [engineViewCallbacks]);

  console.log("Returning EngineScreen");
  return (
    <>
      <View style={props.style}>
        <Button title="Toggle EngineView" onPress={() => { setToggleView(!toggleView) }} />
        <Button title={ xrSession ? "Stop XR" : "Start XR"} onPress={onToggleXr} />
        { !toggleView &&
          <View style={{flex: 1}}>
            {/* { enableSnapshots && 
              <View style ={{flex: 1}}>
                <Button title={"Take Snapshot"} onPress={onSnapshot}/>
                <Image style={{flex: 1}} source={{uri: snapshotData }} />
              </View>
            } */}
            {/* <EngineView style={props.style} camera={camera} onInitialized={onInitialized} /> */}
            <EngineView style={props.style} camera={camera} />
            {/* <Slider style={{position: 'absolute', minHeight: 50, margin: 10, left: 0, right: 0, bottom: 0}} minimumValue={0.2} maximumValue={2} value={defaultScale} onValueChange={setScale} /> */}
          </View>
        }
        { toggleView &&
          <View style={{flex: 1, justifyContent: 'center', alignItems: 'center'}}>
            <Text style={{fontSize: 24}}>EngineView has been removed.</Text>
            <Text style={{fontSize: 12}}>Render loop stopped, but engine is still alive.</Text>
          </View>
        }
      </View>
    </>
  );
};

console.log("Declaring app");
const App = () => {
  console.log("Declaring app setToggleScreen");
  const [toggleScreen, setToggleScreen] = useState(false);

  console.log("Returning app");
  return (
    <>
      <StatusBar barStyle="dark-content" />
      <SafeAreaView style={{flex: 1}}>
        { !toggleScreen &&
          <EngineScreen style={{flex: 1}} />
        }
        { toggleScreen &&
          <View style={{flex: 1, justifyContent: 'center', alignItems: 'center'}}>
            <Text style={{fontSize: 24}}>EngineScreen has been removed.</Text>
            <Text style={{fontSize: 12}}>Engine has been disposed, and will be recreated.</Text>
          </View>
        }
        <Button title="Toggle EngineScreen" onPress={() => { setToggleScreen(!toggleScreen) }} />
      </SafeAreaView>
    </>
  );
};

export default App;

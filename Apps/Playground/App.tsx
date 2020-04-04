/**
 * Generated with the TypeScript template
 * https://github.com/react-native-community/react-native-template-typescript
 *
 * @format
 */

import React, { useState, FunctionComponent } from 'react';
import { SafeAreaView, StatusBar, Button, View, Text, ViewProps } from 'react-native';

import { EngineView, useEngine } from 'react-native-babylon';
import { Scene, Vector3, Mesh, ArcRotateCamera, Engine, Camera, PBRMetallicRoughnessMaterial, Color3, PromisePolyfill } from '@babylonjs/core';

const EngineScreen: FunctionComponent<ViewProps> = (props: ViewProps) => {
  const [toggleView, setToggleView] = useState(false);
  const [camera, setCamera] = useState<Camera>();

  useEngine((engine: Engine) => {
    var scene = new Scene(engine);
    scene.createDefaultCamera(true);
    if (scene.activeCamera != null) {
      (scene.activeCamera as ArcRotateCamera).beta -= Math.PI / 8;
      setCamera(scene.activeCamera);
    }
    scene.createDefaultLight(true);

    const box = Mesh.CreateBox("box", 0.3, scene);
    const mat = new PBRMetallicRoughnessMaterial("mat", scene);
    mat.metallic = 1;
    mat.roughness = 0.5;
    mat.baseColor = Color3.Red();
    box.material = mat;

    scene.beforeRender = function () {
      scene.meshes[0].rotate(Vector3.Up(), 0.005 * scene.getAnimationRatio());
    };
  });

  return (
    <>
      <View style={props.style}>
        { !toggleView &&
          <EngineView style={props.style} camera={camera} />
        }
        { toggleView &&
          <View style={{flex: 1, justifyContent: 'center', alignItems: 'center'}}>
            <Text style={{fontSize: 24}}>EngineView has been removed.</Text>
            <Text style={{fontSize: 12}}>Render loop stopped, but engine is still alive.</Text>
          </View>
        }
        <Button title="Toggle EngineView" onPress={() => { setToggleView(!toggleView) }} />
      </View>
    </>
  );
};

const App = () => {
  const [toggleScreen, setToggleScreen] = useState(false);

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

Unity移行用: プロジェクト概要

このディレクトリに Unity プロジェクト用の最低限のファイルとスクリプトを追加しました。

開き方（短く）:
- Unity Editor 6000.5.4f1 でプロジェクトルートを開いてください。
- StreamingAssets/maps/ に map1.txt, map2.txt, map3.txt を配置済みです。

使い方（簡易）:
- Assets/Scenes が空の場合は新規シーンを作成し、Hierarchy に空の GameObject を作成して MapImporter コンポーネントを追加します。
- MapImporter のプレハブ参照 (wallPrefab, floorPrefab, keyPrefab, playerPrefab, goalPrefab, enemySpawnPrefab) に適切なプレハブを割り当て、ContextMenu -> Generate Map を実行するとシーンにマップが生成されます。

注意点:
- PrefabUtility をエディタ時に使用しているため、エディタ内で Generate Map を行ってください。
- PlayerController / EnemyAI は単純なスタブ実装です。ゲームロジックの移植と微調整が必要です。

# Balling Maze - Unity移行ガイド

このフォルダには Unity への移行用のスクリプトが格納されています。

推奨環境
- Unity 2022.x LTS 以上（URP を使う場合は URP のセットアップを推奨）
- 新しい Input System を使う場合は Package Manager で追加してください。

セットアップ手順（最小）
1. Unity Hub で新規プロジェクトを作成（3D テンプレート、URP でも可）。
2. このリポジトリの `UnityProject/Assets` 下のファイルをプロジェクトの `Assets/` にコピーします。
3. `StreamingAssets/maps/` フォルダをプロジェクトの `Assets/StreamingAssets/maps/` にコピーし、map1.txt 等を配置します。
4. シーンに空の GameObject を作成し、`MapImporter` スクリプトをアタッチして `mapFile` を指定、必要なら prefab を割り当てる。
5. `PlayerController` は CharacterController をアタッチした GameObject に割り当てる。タグを `Player` に設定すると Enemy が自動でターゲットを見つけます。
6. カメラに `CameraController` をアタッチし、Player を `player` に割り当てる（自動検索されます）。

注意
- サウンドは Unity の AudioSource に置き換えてください。
- プレハブやマテリアルはプロジェクト内で作成・調整してください。



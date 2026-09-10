using System.IO;
using UnityEngine;

[ExecuteInEditMode]
public class MapImporter : MonoBehaviour
{
    public string mapFile = "map1.txt";
    public float cellSize = 1.0f;
    public GameObject wallPrefab;
    public GameObject floorPrefab;
    public GameObject keyPrefab;
    public GameObject playerPrefab;
    public GameObject goalPrefab;
    public GameObject enemySpawnPrefab;

    [ContextMenu("Clear Map")]
    public void ClearMap()
    {
        var children = new System.Collections.Generic.List<GameObject>();
        for (int i = transform.childCount - 1; i >= 0; --i)
        {
            var c = transform.GetChild(i).gameObject;
            children.Add(c);
        }
        foreach (var c in children) DestroyImmediate(c);
    }

    [ContextMenu("Generate Map")]
    public void GenerateMap()
    {
        ClearMap();
        string mapsDir = Path.Combine(Application.streamingAssetsPath, "maps");
        string path = Path.Combine(mapsDir, mapFile);
        if (!File.Exists(path))
        {
            Debug.LogError($"Map file not found: {path}");
            return;
        }

        var lines = File.ReadAllLines(path);
        int height = lines.Length;
        int width = 0;
        foreach (var l in lines) if (l.Length > width) width = l.Length;

        // create floor
        if (floorPrefab)
        {
            var f = PrefabUtilityInstantiate(floorPrefab);
            f.transform.parent = transform;
            f.transform.localPosition = new Vector3(width * cellSize / 2f, 0f, height * cellSize / 2f);
            f.transform.localScale = new Vector3(width * cellSize, 1f, height * cellSize);
        }

        for (int r = 0; r < height; ++r)
        {
            var row = lines[r];
            for (int c = 0; c < row.Length; ++c)
            {
                char ch = row[c];
                Vector3 pos = new Vector3(c * cellSize + cellSize * 0.5f, 0f, r * cellSize + cellSize * 0.5f);
                switch (ch)
                {
                    case '#':
                        if (wallPrefab) { var w = PrefabUtilityInstantiate(wallPrefab); w.transform.parent = transform; w.transform.localPosition = pos; }
                        break;
                    case 'S':
                        if (playerPrefab) { var p = PrefabUtilityInstantiate(playerPrefab); p.transform.parent = transform; p.transform.localPosition = pos; }
                        break;
                    case 'G':
                        if (goalPrefab) { var g = PrefabUtilityInstantiate(goalPrefab); g.transform.parent = transform; g.transform.localPosition = pos; }
                        break;
                    case 'a': case 'b': case 'c':
                        if (keyPrefab) { var k = PrefabUtilityInstantiate(keyPrefab); k.transform.parent = transform; k.transform.localPosition = pos; }
                        break;
                    case 'A': case 'B': case 'C':
                        if (enemySpawnPrefab) { var s = PrefabUtilityInstantiate(enemySpawnPrefab); s.transform.parent = transform; s.transform.localPosition = pos; }
                        break;
                }
            }
        }

        Debug.Log($"Generated map {mapFile} ({width}x{height})");
    }

    // helper to instantiate prefab in editor or runtime
    private GameObject PrefabUtilityInstantiate(GameObject prefab)
    {
        if (Application.isPlaying) return Instantiate(prefab);
#if UNITY_EDITOR
        // PrefabUtility requires UnityEditor namespace
        return (GameObject)UnityEditor.PrefabUtility.InstantiatePrefab(prefab);
#else
        return Instantiate(prefab);
#endif
    }
}

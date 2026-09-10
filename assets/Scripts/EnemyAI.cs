using UnityEngine;

public class EnemyAI : MonoBehaviour
{
    public Transform target;
    public float speed = 2f;
    public float wallAdjMultiplier = 3f;

    void Update()
    {
        if (!target) return;
        Vector3 dir = (target.position - transform.position);
        dir.y = 0;
        float dist = dir.magnitude;
        if (dist < 0.001f) return;
        dir.Normalize();

        // simplistic wall-adjacent speed: if near any collider within small radius, increase speed
        float sp = speed;
        Collider[] hits = Physics.OverlapSphere(transform.position, 0.6f);
        foreach (var h in hits)
        {
            if (h.gameObject != gameObject && h.gameObject.tag == "Wall") { sp *= wallAdjMultiplier; break; }
        }

        transform.position += dir * sp * Time.deltaTime;
    }
}

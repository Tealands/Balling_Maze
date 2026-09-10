using UnityEngine;

public class CameraController : MonoBehaviour
{
    public Transform player;
    public bool firstPerson = false;
    public float mouseSensitivity = 2.0f;
    public float distance = 6.0f;
    public float pitch = 10.0f;

    private float yaw = 0.0f;
    private float currentDistance;

    void Start()
    {
        if (player == null && GameObject.FindWithTag("Player") != null)
        {
            player = GameObject.FindWithTag("Player").transform;
        }
        currentDistance = distance;
        Cursor.lockState = CursorLockMode.None; // not locking by default
    }

    void Update()
    {
        // toggle camera mode
        if (Input.GetKeyDown(KeyCode.Q)) firstPerson = !firstPerson;

        // mouse look when right button held
        if (Input.GetMouseButton(1))
        {
            float dx = Input.GetAxis("Mouse X");
            float dy = Input.GetAxis("Mouse Y");
            yaw += dx * mouseSensitivity;
            pitch -= dy * mouseSensitivity;
            pitch = Mathf.Clamp(pitch, -80f, 80f);
        }

        // wheel for third-person distance
        float wheel = Input.GetAxis("Mouse ScrollWheel");
        if (Mathf.Abs(wheel) > 0.001f)
        {
            currentDistance -= wheel * 2.0f;
            currentDistance = Mathf.Clamp(currentDistance, 2.0f, 15.0f);
        }
    }

    void LateUpdate()
    {
        if (player == null) return;

        if (firstPerson)
        {
            Vector3 eye = player.position + new Vector3(0, 0.6f, 0);
            transform.position = eye;
            transform.rotation = Quaternion.Euler(pitch, yaw, 0);
        }
        else
        {
            Vector3 dir = new Vector3(Mathf.Cos(yaw * Mathf.Deg2Rad), 0, Mathf.Sin(yaw * Mathf.Deg2Rad));
            Vector3 camPos = player.position - dir * currentDistance + new Vector3(0, 3.0f, 0);
            transform.position = camPos;
            transform.LookAt(player.position + new Vector3(0,0.5f,0));
        }
    }
}

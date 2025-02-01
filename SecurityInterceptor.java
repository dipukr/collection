package in.hertz.samast.config;

import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.Date;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.ResponseEntity;
import org.springframework.kafka.core.KafkaTemplate;
import org.springframework.stereotype.Component;
import org.springframework.web.client.HttpClientErrorException;
import org.springframework.web.client.RestClientException;
import org.springframework.web.client.RestTemplate;
import org.springframework.web.servlet.HandlerInterceptor;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.hertz.sel.domain.UserSlim;
import com.hertz.sel.util.JWTToken;
import com.hertz.sel.util.UserContext;

import in.hertz.samast.domain.ActivityLogBO;
import in.hertz.samast.entity.mongo.ActivityLog;
import in.hertz.samast.entity.mongo.ActivityLogMaster;
import in.hertz.samast.mongodb.ActivityLogMasterRepository;
import in.hertz.samast.service.ActivityLogService;

@Component
public class SecurityInterceptor implements HandlerInterceptor {

	private static final Logger log = LogManager.getLogger(SecurityInterceptor.class);
	private RestTemplate restTemplate = new RestTemplate();

	@Value("${auth.service.context}")
	private String authServiceContext;

	@Value("${activityLog.topic.name}")
	private String activityLogTopicName;

	@Autowired
	private KafkaTemplate<String, String> kt;

	@Autowired
	private ActivityLogService activityLogService;

	@Override
	public boolean preHandle(HttpServletRequest request, HttpServletResponse response, Object handler)
			throws Exception {
		HttpServletRequest req = (HttpServletRequest) request;
		HttpServletResponse resp = (HttpServletResponse) response;
		Cookie[] cookies = req.getCookies();
		for (Cookie cookie : cookies) {
			if (cookie.getName().equalsIgnoreCase("samast")) {
				String authString = cookie.getValue();
				log.debug("Request Cookie: {} = {}", cookie.getName(), cookie.getValue());
				try {
					JWTToken.verifyToken(authString);
					ResponseEntity<UserSlim> restEntity = restTemplate.postForEntity(
							new URI(authServiceContext + "rest/logInRestService/authtokenvalid"), authString,
							UserSlim.class);
					UserSlim user = restEntity.getBody();
					user.setAuthToken(authString);
					UserContext.setUser(user);
					processReqInfo(user, req);
					return true;
				} catch (HttpClientErrorException e) {
					log.debug("Error in validating auth token. Error Msg: {}", e.getMessage());
				} catch (RestClientException | URISyntaxException e) {
					log.error(e.getMessage(), e);
				}
			}
		}
		return true;
	}

	private void processReqInfo(UserSlim userInfo, HttpServletRequest req) throws IOException {

		Date dateTime = new Date();
		String user = userInfo.getFullName();
		String userID = userInfo.getEmailID();
//		String ipAddress = req.getRemoteAddr();
		String requestURI = req.getRequestURI();
		String ipAddress = req.getHeader("X-Forwarded-For");
		String actionDescription = "";
		String reqBody = "";
		
//		if ("POST".equalsIgnoreCase(req.getMethod())) {
//			InputStream requestInputStream = req.getInputStream();			
//			byte[] cachedBody = StreamUtils.copyToByteArray(requestInputStream);
//			ByteArrayInputStream byteArrayInputStream = new ByteArrayInputStream(cachedBody);
//			BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(byteArrayInputStream));
//			
////			InputStreamReader inputStreamReader = new InputStreamReader(requestInputStream);
////			BufferedReader bufferedReader = new BufferedReader(inputStreamReader);
//			
////			StringBuffer sb = new StringBuffer();
////			String str ;
////			while((str = bufferedReader.readLine()) != null){
////				sb.append(str) ;
////			}
////			reqBody = sb.toString();
//			reqBody = bufferedReader.lines().collect(Collectors.joining(System.lineSeparator()));
//		}
		ActivityLogBO activityLog = new ActivityLogBO();

		activityLog.setDateTime(dateTime);
		activityLog.setUser(user);
		activityLog.setUserID(userID);
		activityLog.setIpAddress(ipAddress);
		activityLog.setRequestURI(requestURI);
		activityLog.setActionDescription(actionDescription);
		activityLog.setMethod(req.getMethod());
		activityLog.setReqBody(reqBody);

		ObjectMapper mapper = new ObjectMapper();
		String json = mapper.writeValueAsString(activityLog);

		kt.send(activityLogTopicName, json);
	}
}
